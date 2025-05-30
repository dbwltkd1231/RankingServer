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

	void Session::Process(HANDLE mIocpHandle, 
		std::shared_ptr<tbb::concurrent_map<int, std::shared_ptr<Client>>> clientMap, 
		std::shared_ptr<Utility::LockFreeCircleQueue<CustomOverlapped*>> overlappedQueue,
		std::function<void(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer)> receiveCallback
	)
	{
		CustomOverlapped* overlapped = nullptr;
		DWORD bytesTransferred = 0; 
		ULONG_PTR completionKey = 0; 

		while (mActive)
		{
			BOOL result = GetQueuedCompletionStatus(mIocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

			if (!result)
			{
				int errorCode = WSAGetLastError();
				Utility::Debug("Network", "Session", "GetQueuedCompletionStatus Failed! Error Code: " + std::to_string(errorCode));
				continue;
			}
			Utility::Debug("Network", "Session", "Message Recieve Check !");
			switch (overlapped->mOperationType)
			{
			case OperationType::OP_ACCEPT:
			{
				MessageHeader* receivedHeader = reinterpret_cast<MessageHeader*>(overlapped->mWsabuf[0].buf);

				auto clientFinder = clientMap.get()->find(receivedHeader->mSocketId);

				if (clientFinder == clientMap->end())
				{
					Utility::Debug("Network", "Session", "Accept Sokcet Find Fail !");
					continue;
				}

				auto client = clientFinder->second;

				std::string log = std::to_string(receivedHeader->mSocketId) + " Socket Connected Success !";
				Utility::Debug("Network", "Session", log);

				client->ReceiveReady();

				break;
			}
			case OperationType::OP_RECV:
			{
				auto clientFinder = clientMap.get()->find(completionKey);
				if (clientFinder == clientMap->end())
				{
					Utility::Debug("Network", "Session", "Recv Sokcet Find Fail !");
					continue;
				}

				auto client = clientFinder->second;
				if (bytesTransferred <= 0)
				{
					std::string log = std::to_string(completionKey) + " Socekt Disconneted OR Recv Error";
					Utility::Debug("Network", "Session", log);
				}
				else
				{
					MessageHeader* receivedHeader = reinterpret_cast<MessageHeader*>(overlapped->mWsabuf[0].buf);

					std::string log = std::to_string(completionKey) + " Socket Recv Success";
					Utility::Debug("Network", "Session", log);

					auto socketId = completionKey;
					auto requestBodySize = ntohl(receivedHeader->mBodySize);
					auto requestContentsType = ntohl(receivedHeader->mContentsType);
					
					receiveCallback(socketId, requestBodySize, requestContentsType, overlapped->mWsabuf[1].buf);
				}

				client->ReceiveReady();
				break;
			}
			case OperationType::OP_SEND:
			{
				if (bytesTransferred <= 0)
				{
					Utility::Debug("Network", "Session", "Send Error");
				}
				else
				{
					Utility::Debug("Network", "Session", "Send Success");
				}

				break;
			}
			case OperationType::OP_DEFAULT:
			{
				Utility::Debug("Network", "Session", "Default");
				break;
			}
			}

			overlapped->Clear();
			overlappedQueue->push(std::move(overlapped));
		}
	
	}
}