#pragma once
#include<iostream>

#include "flatbuffers/flatbuffers.h"
#include "RANKING_PROTOCOL_generated.h"

//#include"Converter.h"
#include "Log.h"

namespace Business
{
	class RankingServer
	{
	public:
		RankingServer();
		~RankingServer();

		void MessageRead(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer);
		//void MessageWrite();

	private:
		//DatabaseWorker mDatabaseWorker;
	};
}