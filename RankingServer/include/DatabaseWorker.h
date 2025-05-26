#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

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

	private:
		SQLHENV mHenv;
		SQLHDBC mHdbc;
		SQLHSTMT mHstmt;
		redisContext* mRedis;

		std::unordered_set<std::string> mTableNameSet;
		std::unordered_map<std::string, TableType> mTableMap;

		TableType getTableType(const std::string& table);
		void SetCachedData(const std::string table, const std::string key, std::string jsonString, int ttl);
	};
}