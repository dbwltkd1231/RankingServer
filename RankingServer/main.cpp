#pragma once
#include "Hub.h"

#define MY_IP "127.0.0.1"
#define MY_PORT_NUM 9090
#define REDIS_PORT_NUM 6379
#define SOCKET_MAX 10
#define RANKING_UPDATE_INTERVAL_MINUTES 1 

int main()
{
	Business::Hub hub;
	hub.Initialize(MY_IP, MY_PORT_NUM, REDIS_PORT_NUM, SOCKET_MAX);
	hub.Work(RANKING_UPDATE_INTERVAL_MINUTES);
}
