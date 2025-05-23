
#include <iostream>
#include <string>

#include "RankingServer.h"
#include "NetworkManager.h"

#define MY_IP "127.0.0.1"
#define MY_PORT_NUM 9090
#define REDIS_PORT_NUM 6379

int main()
{
	Business::RankingServer rankingServer;

	auto receiveCallback = std::function<void(uint32_t, uint32_t, uint32_t, char*)>(
		std::bind(&Business::RankingServer::MessageRead, std::ref(rankingServer),
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);

	Network::NetworkManager networkManager;
	networkManager.Initialize(MY_PORT_NUM, 10);
	networkManager.Ready(20, receiveCallback);

	while (true)
	{

	}
}
