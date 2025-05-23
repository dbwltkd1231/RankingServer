#pragma once
#include "DatabaseWorker.h"  
#include <iostream>
#include <string>



namespace Business
{
    void DatabaseWorker::Initalize()
    {
        SQLWCHAR sqlState[6], message[256];
        SQLINTEGER nativeError;
        SQLSMALLINT messageLength;

        SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &mHenv);

        ret = SQLSetEnvAttr(mHenv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            SQLGetDiagRecW(SQL_HANDLE_DBC, mHdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);
            std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
            return;
        }
        std::cout << "MSSQL Connect-4 Success" << std::endl;

        //연결 핸들 생성
        ret = SQLAllocHandle(SQL_HANDLE_DBC, mHenv, &mHdbc);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            SQLGetDiagRecW(SQL_HANDLE_DBC, mHdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);
            std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
            return;
        }
        std::cout << "MSSQL Connect-3 Success" << std::endl;

        SQLWCHAR* connStr = (SQLWCHAR*)L"DRIVER={SQL Server};SERVER=DESKTOP-O5SU309\\SQLEXPRESS;DATABASE=MyChat;Trusted_Connection=yes;"; //->windows인증일때 사용


        //DB에 연결
        //->UNICODE 버전
        ret = SQLDriverConnectW(mHdbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            SQLGetDiagRecW(SQL_HANDLE_DBC, mHdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);
            std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
            return;
        }

     //쿼리 실행을 위한 문장 핸들 생성
        ret = SQLAllocHandle(SQL_HANDLE_STMT, mHdbc, &mHstmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cout << "ODBC 오류 발생" << std::endl;
            return;
        }

        SQLSetConnectAttr(mHdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);//자동커밋

        std::cout << "MSSQL Connect Success" << std::endl;

        mTableMap = 
        {
        {"Ranking", TableType::Ranking}
        };
    }

    void DatabaseWorker::Deinitalize()
    {
        clearTableCache("Ranking");
        redisFree(mRedis); 

        SQLFreeHandle(SQL_HANDLE_STMT, mHstmt);
        SQLDisconnect(mHdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, mHdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, mHenv);
    }

    bool DatabaseWorker::DataCaching(std::string ip, int port)
    {
        //이코드 실행전 Redis 서버 실행 필요
        mRedis = redisConnect(ip.c_str(), port);
        if (mRedis == NULL || mRedis->err)
        {
            std::cout << "Redis 연결 실패!" << std::endl;
            return false;
        }
        std::cout << "Redis 연결 성공!" << std::endl;


        SQLWCHAR* tableQuery = (SQLWCHAR*)L"SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_CATALOG = 'MyChat';";
        SQLRETURN ret = SQLExecDirectW(mHstmt, tableQuery, SQL_NTS);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
        {
            std::cout << "쿼리 실행 성공" << std::endl;

            SQLWCHAR tableName[256];
            while (SQLFetch(mHstmt) == SQL_SUCCESS) {
                SQLGetData(mHstmt, 1, SQL_C_WCHAR, tableName, sizeof(tableName), NULL);

                std::cout << "TableName : " << Utility::Converter::StringConvert(std::wstring(tableName)) << std::endl;
                mTableNameSet.insert(Utility::Converter::StringConvert(std::wstring(tableName)));
            }
        }
        else
        {
            std::cout << "쿼리 실행 오류 발생" << std::endl;
        }

        SQLFreeStmt(mHstmt, SQL_CLOSE); // 이전 쿼리 결과 닫기

        for (auto table : mTableNameSet)
        {
            SQLFreeStmt(mHstmt, SQL_CLOSE); // 이전 쿼리 결과 닫기
            auto tableType = getTableType(table);

            switch (tableType)
            {
            case TableType::Unknown:
            {
                std::cout << "Unknown" << std::endl;
                break;
            }
            default:
                break;
            }

            //SQLFreeStmt(mHstmt, SQL_CLOSE); // 이전 쿼리 결과 닫기
        }

        return true;
    }

    TableType DatabaseWorker::getTableType(const std::string& table)
    {
        auto it = mTableMap.find(table);
        return (it != mTableMap.end()) ? it->second : TableType::Unknown;
    }


    void DatabaseWorker::SetCachedData(const std::string table, const std::string key, std::string jsonString, int ttl)
    {
        std::string cacheKey = "table:" + table + ":" + key;
        redisReply* reply = (redisReply*)redisCommand(mRedis, "SET %s %s EX %d", cacheKey.c_str(), jsonString.c_str(), ttl);
    }

    void DatabaseWorker::clearTableCache(const std::string table)
    {
        std::string cacheKey = "table:" + table;

        redisReply* reply = (redisReply*)redisCommand(mRedis, "DEL %s", cacheKey.c_str());
        if (reply == NULL) {
            std::cout << "Redis 삭제 오류!" << std::endl;
            return;
        }

        std::cout << "초기화 완료: " << cacheKey << " 삭제됨!" << std::endl;
        freeReplyObject(reply);
    }

    bool DatabaseWorker::IsKeyExists(const std::string table, const std::string key) {

        std::string cacheKey = table + ":" + key;
        redisReply* reply = (redisReply*)redisCommand(mRedis, "EXISTS %s", cacheKey.c_str());
        if (reply == NULL) {
            std::cout << "Redis 확인 오류!" << std::endl;
            return false;
        }

        bool exists = reply->integer == 1;
        freeReplyObject(reply);
        return exists;
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

        std::cout << " 데이터가 없거나 오류 발생 " << std::endl;
        return "";
    }

    void DatabaseWorker::SaveSQLDatabase(std::string table)
    {
        auto tableFinder = mTableMap.find(table);
        if (tableFinder == mTableMap.end())
        {
            std::cout << "테이블이 존재하지 않습니다." << std::endl;
            return;
        }

        auto tableName = tableFinder->first;
        auto tableType = tableFinder->second;

        int cursor = 0;
        std::vector<std::string> keys;
        redisReply* reply = nullptr;
        do
        {
            reply = (redisReply*)redisCommand(mRedis, "SCAN %d MATCH table:%s:*", cursor, table.c_str());

            if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
                std::cerr << "Redis SCAN 실패!" << std::endl;
                break;
            }

            cursor = std::stoi(reply->element[0]->str);
            redisReply* keyList = reply->element[1];

            if (keyList->type == REDIS_REPLY_ARRAY)
            {
                for (size_t i = 0; i < keyList->elements; i++)
                {
                    if (keyList->element[i] != nullptr && keyList->element[i]->str != nullptr)
                    {
                        keys.push_back(keyList->element[i]->str);
                    }
                }
            }

            freeReplyObject(reply);

        } while (cursor != 0);

        if (keys.empty())
        {
            std::cout << "Redis에 데이터가 없습니다." << std::endl;
            return;
        }

        for (const auto& key : keys)
        {
            reply = (redisReply*)redisCommand(mRedis, "GET %s", key.c_str());
            std::string result;
            if (reply != nullptr && reply->str)
            {
                result = reply->str;

                if (!result.empty())
                {
                    nlohmann::json jsonData = nlohmann::json::parse(result);

                    SQLAllocHandle(SQL_HANDLE_STMT, mHdbc, &mHstmt);

                    switch (tableType)
                    {
                  
                    default:
                        break;
                    }
                }
            }

            freeReplyObject(reply);
        }
    }
}