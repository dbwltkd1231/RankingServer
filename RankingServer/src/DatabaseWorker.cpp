#pragma once
#include "DatabaseWorker.h"
#include "Converter.h"

namespace Business
{
	DatabaseWorker::DatabaseWorker()
	{

	}

	DatabaseWorker::~DatabaseWorker()
	{

	}

	void DatabaseWorker::Initalize()
	{
        SQLWCHAR sqlState[6], message[256];
        SQLINTEGER nativeError;
        SQLSMALLINT messageLength;

        SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &mHenv);

        //ODBC 버전 설정
        ret = SQLSetEnvAttr(mHenv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            SQLGetDiagRecW(SQL_HANDLE_DBC, mHdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);
            std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
            return;
        }
        Utility::Debug("Business", "DatabaseWorker", "MSSQL Version Setting Success");

        //연결 핸들 생성
        ret = SQLAllocHandle(SQL_HANDLE_DBC, mHenv, &mHdbc);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            SQLGetDiagRecW(SQL_HANDLE_DBC, mHdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);
            std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
            return;
        }
        Utility::Debug("Business", "DatabaseWorker", "MSSQL Handle Create Success");

        SQLWCHAR* connStr = (SQLWCHAR*)L"DRIVER={SQL Server};SERVER=DESKTOP-O5SU309\\SQLEXPRESS;DATABASE=RankingServer;Trusted_Connection=yes;"; //->windows인증일때 사용


        //DB에 연결
        //->UNICODE 버전
        ret = SQLDriverConnectW(mHdbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            SQLGetDiagRecW(SQL_HANDLE_DBC, mHdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);

            std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
            return;
        }
        Utility::Debug("Business", "DatabaseWorker", "MSSQL DB Connect Success");

        ret = SQLAllocHandle(SQL_HANDLE_STMT, mHdbc, &mHstmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cout << "ODBC 오류 발생" << std::endl;
            return;
        }
        Utility::Debug("Business", "DatabaseWorker", "MSSQL Querry Handle Create Success");

        SQLSetConnectAttr(mHdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);//자동커밋 세팅

        mTableMap = {
                    {"Score", TableType::Score},
                    {"Ranking", TableType::Ranking}
        };
	}

	void DatabaseWorker::RedisConnect(std::string ip, int port)
	{
        //이코드 실행전 Redis 서버 실행 필요
        mRedis = redisConnect(ip.c_str(), port);
        if (mRedis == NULL || mRedis->err)
        {
            Utility::Debug("Business", "DatabaseWorker", "Redis Connect Fail");
            return;
        }
        Utility::Debug("Business", "DatabaseWorker", "Redis Create Success");
	}

	void DatabaseWorker::DataLoadInSQL()
	{
        SQLWCHAR* tableQuery = (SQLWCHAR*)L"SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_CATALOG = 'RankingServer';";
        SQLRETURN ret = SQLExecDirectW(mHstmt, tableQuery, SQL_NTS);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
        {
            Utility::Debug("Business", "DatabaseWorker", "RankingServer Table Find...");

            SQLWCHAR tableName[256];
            while (SQLFetch(mHstmt) == SQL_SUCCESS) {
                SQLGetData(mHstmt, 1, SQL_C_WCHAR, tableName, sizeof(tableName), NULL);

                std::string log = "Table Find : " + Utility::Converter::StringConvert(std::wstring(tableName));
                Utility::Debug("Business", "DatabaseWorker", log);
                mTableNameSet.insert(Utility::Converter::StringConvert(std::wstring(tableName)));
            }

            Utility::Debug("Business", "DatabaseWorker", "RankingServer Table Find End.");
        }
        else
        {
            Utility::Debug("Business", "DatabaseWorker", "Table Find Query Error");
        }

        SQLFreeStmt(mHstmt, SQL_CLOSE); 

        for (auto table : mTableNameSet)
        {
            SQLFreeStmt(mHstmt, SQL_CLOSE);
            auto tableType = getTableType(table);

            switch (tableType)
            {
            case TableType::Unknown:
            {
                Utility::Debug("Business", "DatabaseWorker", "Unkonwon Data Caching...");
                break;
            }
            case TableType::Score:
            {
                Utility::Debug("Business", "DatabaseWorker", "Score Data Caching...");

                std::wstring queryStr = L"SELECT player_id, score, last_update FROM " + std::wstring(table.begin(), table.end());
                SQLWCHAR* dataQuery = (SQLWCHAR*)queryStr.c_str();
                SQLRETURN dataRet = SQLExecDirectW(mHstmt, dataQuery, SQL_NTS);
                if (dataRet == SQL_SUCCESS || dataRet == SQL_SUCCESS_WITH_INFO)
                {
                    SQLWCHAR id[16];
                    int score;
                    TIMESTAMP_STRUCT last_update;

                    int count = 0;
                    while (SQLFetch(mHstmt) == SQL_SUCCESS)
                    {
                        SQLGetData(mHstmt, 1, SQL_C_WCHAR, &id, sizeof(id), NULL);
                        SQLGetData(mHstmt, 1, SQL_C_LONG, &score, 0, NULL);
                        SQLGetData(mHstmt, 3, SQL_C_TYPE_TIMESTAMP, &last_update, sizeof(TIMESTAMP_STRUCT), NULL);

                        std::string id_String = Utility::Converter::WstringToUTF8(id);
                        std::tm timeinfo = { last_update.second, last_update.minute, last_update.hour, last_update.day, last_update.month - 1, last_update.year - 1900 };
                        std::time_t localTime = std::mktime(&timeinfo); // 로컬 시간 변환
                        std::time_t utcTime = _mkgmtime(&timeinfo); // UTC 기준으로 변환

                        auto scoreJson = Data_Score::toJson(id_String, score, utcTime);
                        std::string jsonString = scoreJson.dump(); // JSON을 문자열로 변환

                        SetCachedData(table, id_String, jsonString, 60); // 60초 TTL 설정);
                        count++;
                    }

                    std::string log = "Score Data Caching " + std::to_string(count) + " Success";
                    Utility::Debug("Business", "DatabaseWorker", log);
                }

                break;
            }

            }
        }

	}

    void DatabaseWorker::SetCachedData(const std::string table, const std::string key, std::string jsonString, int ttl)
    {
        std::string cacheKey = "table:" + table + ":" + key;
        redisReply* reply = (redisReply*)redisCommand(mRedis, "SET %s %s EX %d", cacheKey.c_str(), jsonString.c_str(), ttl);
    }

    TableType DatabaseWorker::getTableType(const std::string& table)
    {
        auto it = mTableMap.find(table);
        return (it != mTableMap.end()) ? it->second : TableType::Unknown;
    }
}