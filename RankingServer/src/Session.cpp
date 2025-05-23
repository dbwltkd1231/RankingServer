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
		DWORD bytesTransferred = 0; //bytesTransferred: ���۵� ����Ʈ ��,
		ULONG_PTR completionKey = 0; //completionKey: �Ϸ� Ű

		while (mActive)
		{
			BOOL result = GetQueuedCompletionStatus(mIocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

			if (result)
			{

			}
		}
	
	}
}