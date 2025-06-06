#pragma once
#include "NetworkManager.h"


namespace Network
{
	NetworkManager::NetworkManager()
	{
		Utility::Debug("Network", "NetworkManager", "NetworkManager Construct");
	}

	NetworkManager::~NetworkManager()
	{
		Utility::Debug("Network", "NetworkManager", "NetworkManager Destruct");
	}

	void NetworkManager::Initialize(u_short port, int socketMax)
	{
		mPort = port;

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		mNumThreads = sysInfo.dwNumberOfProcessors * 2;//CPU 코어 수의 2배 만큼의 Session(스레드) 생성.

		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			Utility::Debug("Network", "NetworkManager", "WSAStartup failed");
			return;
		}


		mListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (mListenSocket == INVALID_SOCKET)
		{
			Utility::Debug("Network", "NetworkManager", "listenSocket Create failed");
			WSACleanup();
			return;
		}

		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(mPort);

		if (bind(mListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			Utility::Debug("Network", "NetworkManager", "bind failed");
			closesocket(mListenSocket);
			WSACleanup();
			return;
		}

		Utility::Debug("Network", "NetworkManager", "bind success");

		if (listen(mListenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			Utility::Debug("Network", "NetworkManager", "listen failed");
			closesocket(mListenSocket);
			WSACleanup();
			return;

		}

		Utility::Debug("Network", "NetworkManager", "listen success");

		mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, mNumThreads);
		if (mIOCPHandle == NULL)
		{
			Utility::Debug("Network", "NetworkManager", "CreateIoCompletionPort failed");
			closesocket(mListenSocket);
			WSACleanup();
			return;
		}
		Utility::Debug("Network", "NetworkManager", "CreateIoCompletionPort success");

		if (!CreateIoCompletionPort((HANDLE)mListenSocket, mIOCPHandle, 0, mNumThreads))
		{
			Utility::Debug("Network", "NetworkManager", "CreateIoCompletionPort failed");
			closesocket(mListenSocket);
			WSACleanup();
			return;
		}

		Utility::Debug("Network", "NetworkManager", "Iocp - Socket Connect");

		GUID guidAcceptEx = WSAID_ACCEPTEX;
		DWORD bytesReceived;

		if (WSAIoctl(mListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidAcceptEx, sizeof(guidAcceptEx), &mAcceptExPointer, sizeof(mAcceptExPointer),
			&bytesReceived, NULL, NULL) == SOCKET_ERROR)
		{
			Utility::Debug("Network", "NetworkManager", "WSAIoctl failed");
			return;
		}

		mOverlappedQueue = std::make_shared<Utility::LockFreeCircleQueue<CustomOverlapped*>>();
		mOverlappedQueue->Construct(200);
		for (int i = 0;i < 200;++i)
		{
			auto overlapped = new CustomOverlapped();
			mOverlappedQueue->push(std::move(overlapped));
		}

		mClientMap = std::make_shared<tbb::concurrent_map<int, std::shared_ptr<Client>>>();
		for (int index = 1;index < socketMax + 1; ++index)
		{
			SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			auto socketSharedPtr = std::make_shared<SOCKET>(newSocket);

			std::shared_ptr<Client> clientSharedPtr = std::make_shared<Client>();
			clientSharedPtr->Initialize(socketSharedPtr, index, mOverlappedQueue);
			mClientMap->insert(std::make_pair(index, clientSharedPtr));
			CreateIoCompletionPort((HANDLE)newSocket, mIOCPHandle, (ULONG_PTR)index, mNumThreads);
		}

		Utility::Debug("Network", "NetworkManager", "Initialize Success !!");
	}

	void NetworkManager::Ready(std::function<void(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer)> receiveCallback)
	{
		for (auto& client : *mClientMap)
		{
			auto clientPtr = client.second;
			clientPtr->AcceptReady(mListenSocket, mAcceptExPointer);
		}

		Utility::Debug("Network", "NetworkManager", "Client AcceptReady");

		mSessionQueue = std::make_shared<Utility::LockFreeCircleQueue<std::shared_ptr<Session>>>();
		mSessionQueue->Construct(mNumThreads + 1);
		for (int i = 0;i < mNumThreads;++i)
		{
			auto sessionPtr = std::make_shared<Session>();
			sessionPtr->Activate();

			std::thread thread([sessionPtr, receiveCallback, this]() { sessionPtr->Process(mIOCPHandle, mClientMap, mOverlappedQueue, receiveCallback); });
			thread.detach();

			mSessionQueue->push(std::move(sessionPtr));

			std::string log = std::to_string(i) + " Session Thread Process Start";
			Utility::Debug("Network", "NetworkManager", log);
		}

		Utility::Debug("Network", "NetworkManager", "Ready Success !!");
	}

	void NetworkManager::Send(int socketId, uint32_t contentsType, char* bodyBuffer, int bodySize)
	{
		auto clientFinder = mClientMap.get()->find(socketId);
		if (clientFinder == mClientMap->end())
		{
			Utility::Debug("Network", "NetworkManager", "Send Failed : Client Not Found");
			return;
		}

		auto header_id = htonl(socketId);
		auto header_body_size = htonl(bodySize);
		auto header_contents_type = htonl(contentsType);

		MessageHeader header(header_id, header_body_size, header_contents_type);

		auto client = clientFinder->second;
		client->Send(header, bodyBuffer, bodySize);
	}
}