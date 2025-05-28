#pragma once
#include <iostream>
#include <memory>


#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>
#include <windows.h>

#include "log.h"
#include "MyPacket.h"
#include "LockFreeCircleQueue.h"

#define BUFFER_SIZE 1024

namespace Network
{
	class Client
	{
	public:
		Client();
		~Client();
		void Initialize(
			std::shared_ptr<SOCKET> clientSocketPtr, 
			int socketID,
			std::shared_ptr<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>> overlappedQueue);
		void Deinitialize();
		bool AcceptReady(SOCKET& listenSocket, LPFN_ACCEPTEX& acceptExPointer);
		void ReceiveReady();
		void Send(const MessageHeader& header, const char* bodyBuffer, int bodySize);

	private:
		int mSocket_ID;
		std::shared_ptr<SOCKET> mClientSocketPtr;
		std::shared_ptr<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>> mOverlappedQueue;

		char* mReceive_HeaderBuffer;
		char* mReceive_BodyBuffer;
		char* mSend_HeaderBuffer;
		char* mSend_BodyBuffer;
	};
}
