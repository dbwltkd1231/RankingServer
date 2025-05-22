
#include <iostream>
#include <string>

#include "NetworkManager.h"

#define MY_IP "127.0.0.1"
#define MY_PORT_NUM 9090
#define REDIS_PORT_NUM 6379

int main()
{
	Network::NetworkManager networkManager;
	networkManager.Initialize(MY_PORT_NUM, 20, 10);

}