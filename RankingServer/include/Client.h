#pragma once
#include <iostream>
#include <memory>

#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>
#include <windows.h>


namespace Network
{
	class Client
	{
	public:
		Client();
		~Client();
		void Initialize(std::shared_ptr<SOCKET> clientSocketPtr, int socketID, int headerSize, int bodySize);
		void Deinitialize();
		bool AcceptReady(SOCKET& listenSocket, LPFN_ACCEPTEX& acceptExPointer);
	private:
		int mSocket_ID;
		std::shared_ptr<SOCKET> mClientSocketPtr;

		int mHeaderSize;
		int mBodySize;
		char* mHeaderBuffer;
		char* mBodyBuffer;
	};
}
