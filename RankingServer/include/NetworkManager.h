#pragma once

#include<iostream>
#include<string>
#include<memory>
#include<thread>

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

		void Ready(int sessionQueueMax);
		void Deinitialize();

		std::shared_ptr<Utility::LockFreeCircleQueue<std::shared_ptr<Session>>> mSessionQueue;
	private:
		SOCKET mListenSocket;
		LPFN_ACCEPTEX mAcceptExPointer = nullptr;
		u_short mPort;
		int mNumThreads;

		tbb::concurrent_map<int, std::shared_ptr<Client>> mClientMap;
		std::shared_ptr<Utility::LockFreeCircleQueue<CustomOverlapped*>> mOverlappedQueue;

	};
}
