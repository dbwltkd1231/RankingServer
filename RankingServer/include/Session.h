#pragma once
#include <iostream>
#include <memory>

#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>
#include <windows.h>

#include "MyPacket.h"

#define BUFFER_SIZE 1024

namespace Network
{
	class Session
	{
	public:
		Session();
		~Session();

		void Activate();
		void Process(HANDLE iocpHandle);
		void Deactivate();

	private:
		bool mActive;
	};
}