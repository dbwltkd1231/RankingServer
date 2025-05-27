#pragma once
#include <iostream>
#include <string>
#include <chrono>

#include "RankingSystem.h"
#include "NetworkManager.h"
#include "DatabaseWorker.h"

namespace Business
{
	class Hub
	{
	public:
		Hub();
		~Hub();
		void Initialize(std::string ip, int port, u_short redisPort, int socketMax);
		void Work(int interval);

	

	private:
		Business::RankingSystem mRankingSystem;
		Business::DatabaseWorker databaseWorker;
		Network::NetworkManager networkManager;

		std::function<void(uint32_t, uint32_t, uint32_t, char*)> mReceiveCallback;
		std::function<void(uint32_t, uint32_t, char*)> mSendCallback;

		void Read(uint32_t socketId, uint32_t bodySize, uint32_t contentType, char* bodyBuffer);
		void Send(uint32_t socketId, uint32_t requestType, char* bodyBuffer);

		void Response_PlayerRanking(uint32_t socketId,std::string playerID);
	};
}