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

        SQLWCHAR* connStr = (SQLWCHAR*)L"DRIVER={SQL Server};SERVER=DESKTOP-O5SU309\\SQLEXPRESS;DATABASE=RankingDatabase;Trusted_Connection=yes;"; //->windows인증일때 사용

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
        SQLWCHAR* tableQuery = (SQLWCHAR*)L"SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_CATALOG = 'RankingDatabase';";
        SQLRETURN ret = SQLExecDirectW(mHstmt, tableQuery, SQL_NTS);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
        {
            Utility::Debug("Business", "DatabaseWorker", "RankingDatabase Table Find...");

            SQLWCHAR tableName[256];
            while (SQLFetch(mHstmt) == SQL_SUCCESS)
            {
                SQLGetData(mHstmt, 1, SQL_C_WCHAR, tableName, sizeof(tableName), NULL);

                std::string log = "Table Find : " + Utility::Converter::StringConvert(std::wstring(tableName));
                Utility::Debug("Business", "DatabaseWorker", log);
            }

            Utility::Debug("Business", "DatabaseWorker", "RankingDatabase Table Find End.");
        }
        else
        {
            Utility::Debug("Business", "DatabaseWorker", "Table Find Query Error");
        }

        ScoreDataLoad();
        RankingDataLoad();

        Utility::Debug("Business", "DatabaseWorker", "Data Load from SQL and Caching to Redis Success");
	}

    void DatabaseWorker::ScoreDataLoad()
    {
        SQLFreeStmt(mHstmt, SQL_CLOSE);
        std::string tableName = "Score";
        Utility::Debug("Business", "DatabaseWorker", "Score Data Caching...");

        std::wstring queryStr = L"SELECT player_id, score, last_update FROM " + std::wstring(tableName.begin(), tableName.end());
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
                SQLGetData(mHstmt, 2, SQL_C_LONG, &score, 0, NULL);
                SQLGetData(mHstmt, 3, SQL_C_TYPE_TIMESTAMP, &last_update, sizeof(TIMESTAMP_STRUCT), NULL);

                std::string id_String = Utility::Converter::WstringToUTF8(id);
                std::tm timeinfo = { last_update.second, last_update.minute, last_update.hour, last_update.day, last_update.month - 1, last_update.year - 1900 };
                std::time_t localTime = std::mktime(&timeinfo); // 로컬 시간 변환
                std::time_t utcTime = _mkgmtime(&timeinfo); // UTC 기준으로 변환

                auto scoreJson = Data_Score::toJson(id_String, score, utcTime);
                std::string jsonString = scoreJson.dump(); // JSON을 문자열로 변환

                SetCachedData(tableName, id_String, jsonString, 60); // 60초 TTL 설정);
                count++;
            }

            std::string log = "Score Data Caching " + std::to_string(count) + " Success";
            Utility::Debug("Business", "DatabaseWorker", log);
        }
    }
    
    void DatabaseWorker::RankingUpdate()
    {
        SQLAllocHandle(SQL_HANDLE_STMT, mHdbc, &mHstmt);

        std::wstring queryStr = L"EXEC UpdateRanking;";
        SQLWCHAR* dataQuery = (SQLWCHAR*)queryStr.c_str();
        SQLRETURN ret = SQLExecDirectW(mHstmt, dataQuery, SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            Utility::Debug("Business", "DatabaseWorker", "Ranking Update Error");
        }
        else
        {
            Utility::Debug("Business", "DatabaseWorker", "Ranking Update Success");
        }
    }

    //랭킹데이터는 주기적으로 로드하기위해 코드 분리하였음.
    void DatabaseWorker::RankingDataLoad()
    {
        SQLFreeStmt(mHstmt, SQL_CLOSE);
        std::string tableName = "Ranking";
        Utility::Debug("Business", "DatabaseWorker", "Ranking Data Caching...");

        std::wstring queryStr = L"SELECT TOP 100 rank, player_id, score FROM " + std::wstring(tableName.begin(), tableName.end());
        SQLWCHAR* dataQuery = (SQLWCHAR*)queryStr.c_str();
        SQLRETURN dataRet = SQLExecDirectW(mHstmt, dataQuery, SQL_NTS);

        if (dataRet == SQL_SUCCESS || dataRet == SQL_SUCCESS_WITH_INFO)
        {
            int rank;
            SQLWCHAR id[16];
            int score;

            int count = 0;
            while (SQLFetch(mHstmt) == SQL_SUCCESS)
            {
                SQLGetData(mHstmt, 1, SQL_C_LONG, &rank, 0, NULL);
                SQLGetData(mHstmt, 2, SQL_C_WCHAR, &id, sizeof(id), NULL);
                SQLGetData(mHstmt, 3, SQL_C_LONG, &score, 0, NULL);

                std::string id_String = Utility::Converter::WstringToUTF8(id);

                auto rankingJson = Data_Ranking::toJson(rank, id_String, score);
                std::string jsonString = rankingJson.dump(); // JSON을 문자열로 변환

                SetCachedData(tableName, id_String, jsonString, 600); // SQL에선 rank를 key로 하고있는데 Redis에선 id를 key로...
                count++;
            }

            std::string log = "Ranking Data Caching " + std::to_string(count) + " Success";
            Utility::Debug("Business", "DatabaseWorker", log);
    
        }
    }

    nlohmann::json DatabaseWorker::GetCachedData(const std::string table, const std::string key)
    {
        std::string cacheKey = "table:" + table + ":" + key;

        redisReply* reply = (redisReply*)redisCommand(mRedis, "GET %s", cacheKey.c_str());
        if (reply != NULL && reply->type == REDIS_REPLY_STRING)
        {
            std::string jsonString = reply->str;
            nlohmann::json parsedJson = nlohmann::json::parse(jsonString);
            freeReplyObject(reply);
            return parsedJson;
        }

        std::string log = key + " is not founded in " + table + " Table ";
        Utility::Debug("Business", "DatabaseWorker", key);
        return "";
    }

    void DatabaseWorker::SetCachedData(const std::string table, const std::string key, std::string jsonString, int ttl)
    {
        std::string cacheKey = "table:" + table + ":" + key;
        redisReply* reply = (redisReply*)redisCommand(mRedis, "SET %s %s EX %d", cacheKey.c_str(), jsonString.c_str(), ttl);

        std::string log = "Set Cached Data : " + cacheKey + " - " + jsonString;
        Utility::Debug("Business", "DatabaseWorker", log);
    }

    void DatabaseWorker::ScoreDataSave()
    {
        std::string tableName = "Score";
        int TIMESTAMP_STR_LENGTH = 19;

        std::wstring query = Utility::Converter::ConvertToSQLWCHAR(
            "MERGE INTO Score AS target "
            "USING (VALUES (?, ?, ?)) AS source(player_id, score, last_update) "
            "ON target.player_id = source.player_id "
            "WHEN MATCHED THEN "
            "UPDATE SET target.score = source.score, target.last_update = source.last_update "
            "WHEN NOT MATCHED THEN "
            "INSERT (player_id, score, last_update) VALUES (source.player_id, source.score, source.last_update);"
        );

        int cursor = 0;
        do {
            redisReply* reply = (redisReply*)redisCommand(mRedis, "SCAN %d MATCH table:%s:*", cursor, tableName.c_str());
            if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
                std::cerr << "Redis SCAN 실패!" << std::endl;
                return;
            }

            cursor = std::stoi(reply->element[0]->str); // 다음 SCAN 커서 업데이트
            redisReply* keyList = reply->element[1];

            if (keyList->type == REDIS_REPLY_ARRAY) 
            {
                for (size_t i = 0; i < keyList->elements; i++)
                {
                    if (keyList->element[i] && keyList->element[i]->str) 
                    {
                        std::string key = keyList->element[i]->str;
                        redisReply* replyData = (redisReply*)redisCommand(mRedis, "GET %s", key.c_str());

                        if (replyData && replyData->type == REDIS_REPLY_STRING)
                        {
                            SQLFreeHandle(SQL_HANDLE_STMT, mHstmt);
                            SQLAllocHandle(SQL_HANDLE_STMT, mHdbc, &mHstmt);
                            SQLRETURN retPrepare = SQLPrepare(mHstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
                            if (retPrepare != SQL_SUCCESS && retPrepare != SQL_SUCCESS_WITH_INFO)
                            {
                                std::cerr << "SQLPrepare 실패! 오류 코드: " << retPrepare << std::endl;
                                break;
                            }

                            std::string result = replyData->str;
                            nlohmann::json jsonData = nlohmann::json::parse(result);

                            std::string player_id = jsonData["player_id"];
                            int score = jsonData["score"];

                            std::time_t last_update = jsonData["last_update"];
                            std::tm timeinfo;
                            gmtime_s(&timeinfo, &last_update);
                            std::ostringstream oss;
                            oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
                            std::string timestampStr = oss.str();

                            SQLBindParameter(mHstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 16, 0, (SQLPOINTER)player_id.c_str(), player_id.length(), nullptr);
                            SQLBindParameter(mHstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 16, 0, &score, 0, nullptr);
                            SQLBindParameter(mHstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, TIMESTAMP_STR_LENGTH, 0, (SQLPOINTER)timestampStr.c_str(), timestampStr.length(), nullptr);

                            SQLRETURN ret = SQLExecute(mHstmt);
                            if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
                            {
                                std::cerr << "SQL 실행 실패! 오류 코드: " << ret << std::endl;
                            }
                        }
                    }
                }
            }
        } while (cursor != 0);  // cursor가 0이면 SCAN 종료

        Utility::Debug("Business", "DatabaseWorker", "Score Data Save to SQL");
    }
}