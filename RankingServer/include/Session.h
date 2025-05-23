#pragma once
#include <iostream>
#include <memory>

#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>
#include <windows.h>

#include <oneapi/tbb/concurrent_map.h>

#include "Log.h"
#include "MyPacket.h"
#include "Client.h"

#define BUFFER_SIZE 1024

namespace Network
{
	class Session
	{
	public:
		Session();
		~Session();

		void Activate();
		void Process(HANDLE iocpHandle, std::shared_ptr<tbb::concurrent_map<int, std::shared_ptr<Client>>> clientMap, std::shared_ptr<Utility::LockFreeCircleQueue<CustomOverlapped*>> overlappedQueue);
		void Deactivate();

	private:
		bool mActive;
	};
}