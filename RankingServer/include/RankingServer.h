#pragma once

#include "flatbuffers/flatbuffers.h"
#include "RANKING_PROTOCOL_generated.h"

//#include "DatabaseWorker.h"
#include "Log.h"

namespace Business
{
	class RankingServer
	{
	public:
		RankingServer();
		~RankingServer();

		void DataLoad(std::string ip, int port);
		void MessageRead(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer);
		//void MessageWrite();

	private:
		//DatabaseWorker mDatabaseWorker;
	};
}