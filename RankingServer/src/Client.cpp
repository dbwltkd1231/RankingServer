#pragma once
#include "Client.h"

namespace Network
{
	Client::Client()
	{
		mSocket_ID = -1;
		mClientSocketPtr = nullptr;

	}
	Client::~Client()
	{

	}

	void Client::Initialize(std::shared_ptr<SOCKET> clientSocketPtr, int socketID, int headerSize, int bodySize)
	{
		mHeaderSize = headerSize;
		mBodySize = bodySize;

		mHeaderBuffer = new char[mHeaderSize];
		mBodyBuffer = new char[mBodySize];

		mClientSocketPtr = clientSocketPtr;
		mSocket_ID = socketID;
	}

	void Client::Deinitialize()
	{
		delete[] mHeaderBuffer;
		delete[] mBodyBuffer;
		mHeaderBuffer = nullptr;
		mBodyBuffer = nullptr;
		mHeaderSize = 0;
		mBodySize = 0;

		mClientSocketPtr = nullptr;
	}

	bool Client::AcceptReady(SOCKET& listenSocket, LPFN_ACCEPTEX& acceptExPointer)
	{
		//재활용된 소켓의 상태검증
		int errorCode;
		int errorCodeSize = sizeof(errorCode);
		getsockopt(*mClientSocketPtr, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorCodeSize);
		if (errorCode != 0)
		{
			std::cerr << "Socket error detected: " << errorCode << std::endl;
			return false;
		}

	}
}