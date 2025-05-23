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

		auto overlapped = mOverlappedQueue->pop();
		overlapped->mOperationType = Network::OperationType::OP_ACCEPT;

		memset(mReceive_HeaderBuffer, 0, sizeof(MessageHeader));
		memset(mReceive_BodyBuffer, 0, BUFFER_SIZE);

		overlapped->Initialize(mReceive_HeaderBuffer, mReceive_BodyBuffer, sizeof(MessageHeader), BUFFER_SIZE);
		DWORD bytesReceived = 0;
		bool result = acceptExPointer(listenSocket, *mClientSocketPtr, overlapped->wsabuf[1].buf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &bytesReceived, (CustomOverlapped*)&(*overlapped));
	}
}