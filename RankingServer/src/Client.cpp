#pragma once
#include "Client.h"

namespace Network
{
	Client::Client()
	{
		mSocket_ID = -1;
		mClientSocketPtr = nullptr;

		mReceive_HeaderBuffer = new char[sizeof(MessageHeader)];
		mReceive_BodyBuffer = new char[BUFFER_SIZE];

		mSend_HeaderBuffer = new char[sizeof(MessageHeader)];
		mSend_BodyBuffer = new char[BUFFER_SIZE];

	}
	Client::~Client()
	{
		mSocket_ID = -1;
		mClientSocketPtr = nullptr;

		delete[] mReceive_HeaderBuffer;
		delete[] mReceive_BodyBuffer;

		delete[] mSend_HeaderBuffer;
		delete[] mSend_BodyBuffer;
	}

	void Client::Initialize(
		std::shared_ptr<SOCKET> clientSocketPtr,
		int socketID,
		std::shared_ptr<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>> overlappedQueue)
	{
		mClientSocketPtr = clientSocketPtr;
		mOverlappedQueue = overlappedQueue;
		mSocket_ID = socketID;
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

		memset(mReceive_HeaderBuffer, 0, sizeof(MessageHeader));
		memset(mReceive_BodyBuffer, 0, BUFFER_SIZE);

		auto overlapped = mOverlappedQueue->pop();
		overlapped->mOperationType = Network::OperationType::OP_ACCEPT;

		MessageHeader newHeader(mSocket_ID, 0, 0);
		std::memcpy(mReceive_HeaderBuffer, &newHeader, sizeof(MessageHeader));
		overlapped->SetHeader(mReceive_HeaderBuffer, sizeof(MessageHeader));
		overlapped->SetBody(mReceive_BodyBuffer, BUFFER_SIZE);
		overlapped->hEvent = NULL;
		DWORD bytesReceived = 0;
		bool result = acceptExPointer(listenSocket, *mClientSocketPtr, overlapped->wsabuf[1].buf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &bytesReceived, (CustomOverlapped*)&(*overlapped));

		std::string log;
		errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
		{
			log = std::to_string(mSocket_ID) + "AcceptEx 실패! 오류 코드: " + std::to_string(errorCode);
		}
		else
		{
			log = std::to_string(mSocket_ID) + " Socket Accept Ready";
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
		overlapped->mOperationType = Network::OperationType::OP_RECV;

		memset(mReceive_HeaderBuffer, 0, sizeof(MessageHeader));
		memset(mReceive_BodyBuffer, 0, BUFFER_SIZE);
		
		MessageHeader newHeader(mSocket_ID, 0, 0);
		std::memcpy(mReceive_HeaderBuffer, &newHeader, sizeof(MessageHeader));
		overlapped->SetHeader(mReceive_HeaderBuffer, sizeof(MessageHeader));
		overlapped->SetBody(mReceive_BodyBuffer, BUFFER_SIZE);
		overlapped->hEvent = NULL;

		DWORD flags = 0;
		int result = WSARecv(*mClientSocketPtr, overlapped->wsabuf, 2, nullptr, &flags, &*overlapped, nullptr);

		std::string log;
		errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
		{
			log = std::to_string(mSocket_ID) + "WSARecv 실패! 오류 코드: " + std::to_string(errorCode);
		}
		else
		{
			log = std::to_string(mSocket_ID) + " Socket Receive Ready";
		}

		Utility::Debug("Network", "Client", log);
	}

	void Client::Send(const MessageHeader& header, const char* bodyBuffer, int bodySize)
	{
		if (mClientSocketPtr == nullptr || *mClientSocketPtr == INVALID_SOCKET)
		{
			Utility::Debug("Network", "Client", "Invalid Socket Pointer");
			return;
		}

		memset(mSend_HeaderBuffer, 0, sizeof(MessageHeader));
		memset(mSend_BodyBuffer, 0, BUFFER_SIZE);

		std::memcpy(mSend_HeaderBuffer, &header, sizeof(MessageHeader));
		std::memcpy(mSend_BodyBuffer, bodyBuffer, bodySize);

		auto overlapped = mOverlappedQueue->pop();
		overlapped->mOperationType = Network::OperationType::OP_SEND;
		overlapped->SetHeader(mSend_HeaderBuffer, sizeof(MessageHeader));
		overlapped->SetBody(mSend_BodyBuffer, bodySize);
		overlapped->hEvent = NULL;

		DWORD flags = 0;
		int result = WSASend(*mClientSocketPtr, overlapped->wsabuf, 2, nullptr, flags, &*overlapped, nullptr);
		int errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::string log = std::to_string(mSocket_ID) + "WSASend 실패! 오류 코드: " + std::to_string(errorCode);
			Utility::Debug("Network", "Client", log);
			return;
		}

		Utility::Debug("Network", "Client", "Socket Send Ready");
	}
}