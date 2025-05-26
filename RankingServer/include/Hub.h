#pragma once
#include <iostream>
#include <string>
#include <chrono>

#include "RankingServer.h"
#include "NetworkManager.h"
#include "DatabaseWorker.h"

namespace Business
{
	class Hub
	{
	public:
		Hub();
		~Hub();
		void Initialize(std::string ip, int port, u_short redisPort, int socketMax, int sessionQueueMax);
		void Work(int interval);
	private:
		Business::RankingServer rankingServer;
		Business::DatabaseWorker databaseWorker;
		Network::NetworkManager networkManager;

		std::function<void(uint32_t, uint32_t, uint32_t, char*)> mReceiveCallback;

		void Response_SaveScore(std::string playerID, int score, long last_update);
		void Response_PlayerRanking(std::string playerID);
	};
}