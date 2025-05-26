#pragma once
#include <iostream>
#include <string>
#include <chrono>

#include "RankingServer.h"
#include "NetworkManager.h"
#include "DatabaseWorker.h"

#define MY_IP "127.0.0.1"
#define MY_PORT_NUM 9090
#define REDIS_PORT_NUM 6379
#define SOCKET_MAX 10
#define THREAD_NUM 20
#define UPDATE_INTERVAL 1 

int main()
{
	Business::RankingServer rankingServer;
	Business::DatabaseWorker databaseWorker;

	databaseWorker.Initalize();
	databaseWorker.RedisConnect(MY_IP, REDIS_PORT_NUM);
	databaseWorker.DataLoadInSQL();

	auto receiveCallback = std::function<void(uint32_t, uint32_t, uint32_t, char*)>(
		std::bind(&Business::RankingServer::MessageRead, std::ref(rankingServer),
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);

	Network::NetworkManager networkManager;
	networkManager.Initialize(MY_PORT_NUM, SOCKET_MAX);
	networkManager.Ready(THREAD_NUM, receiveCallback);

	auto tmp = databaseWorker.GetCachedData("Ranking", "player3");
	if (tmp == "")
	{
		Utility::Debug("RankingServer", "Main", "player3 not founded");
	}
	else
	{
		Business::Data_Ranking data_Ranking(tmp);
		std::string dataCheck = "player_id : " + data_Ranking.playerId + " rank : " + std::to_string(data_Ranking.rank);
		Utility::Debug("RankingServer", "Main", dataCheck);
	}

	std::string log = std::to_string(UPDATE_INTERVAL) + "분 경과, 데이터 저장 및 랭킹 업데이트 시작";

	while (true)
	{
		Utility::Debug("RankingServer", "Main", "랭킹 업데이트 대기중...");
		std::this_thread::sleep_for(std::chrono::minutes(UPDATE_INTERVAL));
		Utility::Debug("RankingServer", "Main", log);

		databaseWorker.ScoreDataSave();
		databaseWorker.RankingUpdate();
		databaseWorker.RankingDataLoad();
	}
}
