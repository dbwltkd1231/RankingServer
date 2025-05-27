#pragma once

#include<iostream>
#include<string>
#include<memory>
#include<thread>
#include <functional>

#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>
#include <windows.h>

#include <oneapi/tbb/concurrent_map.h>

#include "LockFreeCircleQueue.h"
#include "Client.h"
#include "MyPacket.h"
#include "Session.h"
#include "Log.h"

#define BUFFER_SIZE 1024

namespace Network
{

	class NetworkManager
	{
	public:
		HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

		NetworkManager();
		~NetworkManager();

		void Initialize(u_short port, int socketMax);

		void Ready(std::function<void(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer)> receiveCallback);
		void Send(int socketId, uint32_t contentsType, const char* bodyBuffer, int bodySize);
		void Deinitialize();

		std::shared_ptr<Utility::LockFreeCircleQueue<std::shared_ptr<Session>>> mSessionQueue;
	private:
		SOCKET mListenSocket;
		LPFN_ACCEPTEX mAcceptExPointer = nullptr;
		u_short mPort;
		int mNumThreads;

		std::shared_ptr<tbb::concurrent_map<int, std::shared_ptr<Client>>> mClientMap;
		std::shared_ptr<Utility::LockFreeCircleQueue<CustomOverlapped*>> mOverlappedQueue;
	};
}
