#pragma once
#include <iostream>
#include <memory>

#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>
#include <windows.h>

#include "MyPacket.h"

namespace Network
{
	class Session
	{
	public:
		Session();
		~Session();

		void Initialize();
		void Deinitialize();


	private:

		DWORD mBytesTransferred;
		ULONG_PTR mCompletionKey;

		bool mActive;

		void Process();
	};
}