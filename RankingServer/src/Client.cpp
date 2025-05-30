#pragma once
#include "Client.h"

namespace Network
{
	Client::Client()
	{
		mSocketId = -1;
		mClientSocketPtr = nullptr;
	}
	Client::~Client()
	{
		mSocketId = -1;
		mClientSocketPtr = nullptr;
	}

	void Client::Initialize(
		std::shared_ptr<SOCKET> clientSocketPtr,
		int socketID,
		std::shared_ptr<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>> overlappedQueue)
	{
		mClientSocketPtr = clientSocketPtr;
		mOverlappedQueue = overlappedQueue;
		mSocketId = socketID;
	}

	void Client::Deinitialize()
	{
		mClientSocketPtr = nullptr;
	}

	bool Client::AcceptReady(SOCKET& listenSocket, LPFN_ACCEPTEX& acceptExPointer)
	{
		int errorCode;
		int errorCodeSize = sizeof(errorCode);
		getsockopt(*mClientSocketPtr, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorCodeSize);
		if (errorCode != 0)
		{
			std::cerr << "Socket error detected: " << errorCode << std::endl;
			return false;
		}

		auto overlapped = mOverlappedQueue->pop();
		overlapped->Clear();
		overlapped->SetOperationType(Network::OperationType::OP_ACCEPT);
		MessageHeader newHeader(mSocketId, 0, 0);
		overlapped->SetHeader(newHeader);

		DWORD bytesReceived = 0;
		bool result = acceptExPointer(listenSocket, *mClientSocketPtr, overlapped->mWsabuf[1].buf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &bytesReceived, (CustomOverlapped*)&(*overlapped));

		std::string log;
		errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
		{
			log = std::to_string(mSocketId) + "AcceptEx 실패! 오류 코드: " + std::to_string(errorCode);
		}
		else
		{
			log = std::to_string(mSocketId) + " Socket Accept Ready";
		}

		Utility::Debug("Network", "Client", log);
	}

	void Client::ReceiveReady()
	{
		int errorCode;
		int errorCodeSize = sizeof(errorCode);
		getsockopt(*mClientSocketPtr, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorCodeSize);
		if (errorCode != 0)
		{
			std::cerr << "Socket error detected: " << errorCode << std::endl;
			return;
		}

		auto overlapped = mOverlappedQueue->pop();
		overlapped->Clear();
		overlapped->SetOperationType(Network::OperationType::OP_RECV);

		DWORD flags = 0;
		int result = WSARecv(*mClientSocketPtr, overlapped->mWsabuf, 2, nullptr, &flags, &*overlapped, nullptr);

		std::string log;
		errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
		{
			log = std::to_string(mSocketId) + "WSARecv 실패! 오류 코드: " + std::to_string(errorCode);
		}
		else
		{
			log = std::to_string(mSocketId) + " Socket Receive Ready";
		}

		Utility::Debug("Network", "Client", log);
	}

	void Client::Send(const MessageHeader& header, char* bodyBuffer, int bodySize)
	{
		if (mClientSocketPtr == nullptr || *mClientSocketPtr == INVALID_SOCKET)
		{
			Utility::Debug("Network", "Client", "Invalid Socket Pointer");
			return;
		}

		auto overlapped = mOverlappedQueue->pop();
		overlapped->Clear();
		overlapped->SetOperationType(Network::OperationType::OP_SEND);
		overlapped->SetHeader(header);
		overlapped->SetBody(bodyBuffer, bodySize);

		DWORD flags = 0;
		int result = WSASend(*mClientSocketPtr, overlapped->mWsabuf, 2, nullptr, flags, &*overlapped, nullptr);
		int errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::string log = std::to_string(mSocketId) + "WSASend 실패! 오류 코드: " + std::to_string(errorCode);
			Utility::Debug("Network", "Client", log);
			return;
		}

		Utility::Debug("Network", "Client", "Socket Send Ready");
	}
}