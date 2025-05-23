#pragma once
#include "Session.h"

namespace Network
{
	Session::Session()
	{
		mActive = false;
	}

	Session::~Session()
	{
		mActive = false;
	}

	void Session::Activate()
	{
		mActive = true;
	}

	void Session::Deactivate()
	{
		mActive = false;
	}

	void Session::Process(HANDLE mIocpHandle)
	{
		CustomOverlapped* overlapped = nullptr;
		DWORD bytesTransferred = 0; //bytesTransferred: 전송된 바이트 수,
		ULONG_PTR completionKey = 0; //completionKey: 완료 키

		while (mActive)
		{
			BOOL result = GetQueuedCompletionStatus(mIocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

			if (result)
			{

			}
		}
	
	}
}