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
	networkManager.Initialize(MY_PORT_NUM, 10);
	networkManager.Ready(20, receiveCallback);

	while (true)
	{
		Utility::Debug("RankingServer", "Main", "랭킹 업데이트 대기중...");
		std::this_thread::sleep_for(std::chrono::minutes(1));  // 1분 대기
		Utility::Debug("RankingServer", "Main", "1분 경과, 데이터 저장 및 랭킹 업데이트 시작");
		databaseWorker.ScoreDataSave();
		databaseWorker.RankingUpdate();
		databaseWorker.RankingDataLoad();
	}
}
