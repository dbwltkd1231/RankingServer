#pragma once

#include<iostream>
#include<string>
#include<memory>

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

		void Initialize(u_short port, int sessionQueueMax, int socketMax);
		void Deinitialize();

		Utility::LockFreeCircleQueue< std::shared_ptr<Session>> mSessionQueue;
	private:
		SOCKET mListenSocket = INVALID_SOCKET;
		LPFN_ACCEPTEX mAcceptExPointer = nullptr;
		u_short mPort;
		int mNumThreads;

		tbb::concurrent_map<int, std::shared_ptr<Client>> mClientMap;

	};
}
