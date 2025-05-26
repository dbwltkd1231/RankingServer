#pragma once
#include <iostream>
#include <string>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include <hiredis.h>

#include "DataStructure.h"
#include "Log.h"

namespace Business
{
	class DatabaseWorker
	{
	public:
		DatabaseWorker();
		~DatabaseWorker();

		void Initalize();
		void RedisConnect(std::string ip, int port);
		void DataLoadInSQL();
		void ScoreDataSave();
		void RankingUpdate();
		void RankingDataLoad();
	private:
		SQLHENV mHenv;
		SQLHDBC mHdbc;
		SQLHSTMT mHstmt;
		redisContext* mRedis;

		void SetCachedData(const std::string table, const std::string key, std::string jsonString, int ttl);
		void ScoreDataLoad();


	};
}