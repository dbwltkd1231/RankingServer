#include "Hub.h"

namespace Business
{
	Hub::Hub()
	{

	}

	Hub::~Hub()
	{

	}

	void Hub::Initialize(std::string ip, int port, u_short redisPort, int socketMax)
	{
		mDatabaseWorker.Initalize();
		mDatabaseWorker.RedisConnect(ip, redisPort);
		mDatabaseWorker.DataLoadInSQL();

		mReceiveCallback = std::function<void(uint32_t, uint32_t, uint32_t, char*)>(
			[this](uint32_t socketId, uint32_t bodySize, uint32_t contentsType, char* bodyBuffer) {
				this->Read(socketId, bodySize, contentsType, bodyBuffer);
			}
		);

		mSendCallback = std::function<void(uint32_t, uint32_t, char*)>(
			[this](uint32_t socketId, uint32_t requestType, char* bodyBuffer) {
				this->Send(socketId, requestType, bodyBuffer);
			}
		);

		mNetworkManager.Initialize(port, socketMax);
		mNetworkManager.Ready(mReceiveCallback);
		mRankingSystem.Ready(mSendCallback);
	}

	void Hub::Read(uint32_t socketId, uint32_t bodySize, uint32_t contentsType, char* bodyBuffer)
	{
		mRankingSystem.MessageRead(socketId, bodySize, contentsType, bodyBuffer);
	}

	void Hub::Send(uint32_t socketId, uint32_t requestType, char* bodyBuffer)
	{
		auto messageType = static_cast<protocol::MessageContent>(requestType);
		Utility::Debug("Business", "Hub", std::to_string(socketId));

		switch (messageType)
		{
		case protocol::MessageContent_REQUEST_PLAYER_RANKING:
		{
			Utility::Debug("Business", "Hub", "Read : REQUEST_PLAYER_RANKING");

			auto requestPlayerRanking = flatbuffers::GetRoot<protocol::REQUEST_PLAYER_RANKING>(bodyBuffer); // FlatBuffers 버퍼에서 루트 객체 가져오기
			Response_PlayerRanking(socketId, requestPlayerRanking->player_id()->str());
			break;
		}
		case protocol::MessageContent_REQUEST_SAVE_SCORE:
		{
			Utility::Debug("Business", "Hub", "Read : REQUEST_SAVE_SCORE");

			auto  requestSaveScore = flatbuffers::GetRoot<protocol::REQUEST_SAVE_SCORE>(bodyBuffer);
			std::time_t unixDate = static_cast<std::time_t>(requestSaveScore->last_update());
			std::time_t koreaTime = unixDate + (9 * 3600);  // UTC+9 → 한국 시간 변환

			Response_SaveScore(socketId, requestSaveScore->player_id()->str(), requestSaveScore->score(), koreaTime);
			break;
		}
		default:
		{
			Utility::Debug("Business", "Hub", "Read : CONTENTS TYPE ERROR !!!!!");
			break;
		}
		}
	}

	void Hub::Work(int interval)
	{
		std::string log = std::to_string(interval) + "분 경과, 데이터 저장 및 랭킹 업데이트 시작";

		while (true)
		{
			Utility::Debug("RankingServer", "Main", "랭킹 업데이트 대기중...");
			std::this_thread::sleep_for(std::chrono::minutes(interval));
			Utility::Debug("RankingServer", "Main", log);

			mDatabaseWorker.ScoreDataSave();
			mDatabaseWorker.RankingUpdate();
			mDatabaseWorker.RankingDataLoad();
		}
	}

	void Hub::Response_SaveScore(uint32_t socketId, std::string playerID, int newScore, std::time_t unixUpdateDate)
	{
		auto scoreJson = Data_Score::toJson(playerID, newScore, unixUpdateDate);
		std::string jsonString = scoreJson.dump();

		mDatabaseWorker.SetCachedData("Score", playerID, jsonString, 60);

		flatbuffers::FlatBufferBuilder builder;
		auto contentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_SAVE_SCORE);
		auto id = builder.CreateString(playerID);

		builder.Finish(protocol::CreateRESPONSE_SAVE_SCORE(builder, id, true));
		char* bodyBuffer = reinterpret_cast<char*>(builder.GetBufferPointer());
		mNetworkManager.Send(socketId, contentsType, bodyBuffer, builder.GetSize());

		std::string log = "Send RESPONSE_SAVE_SCORE : Success ";
		Utility::Debug("Business", "Hub", log);
	}

	void Hub::Response_PlayerRanking(uint32_t socketId, std::string playerId)
	{
		auto contentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_PLAYER_RANKING);
		auto playerRanking = mDatabaseWorker.GetCachedData("Ranking", playerId);

		flatbuffers::FlatBufferBuilder builder;
		auto id = builder.CreateString(playerId);
		int currentScore = 0;
		if (playerRanking == "")
		{
			builder.Finish(protocol::CreateRESPONSE_PLAYER_RANKING(builder, id, currentScore, 0, false, true));
		}
		else
		{
			Business::Data_Ranking data_Ranking(playerRanking);
			currentScore = data_Ranking.mScore;
			builder.Finish(protocol::CreateRESPONSE_PLAYER_RANKING(builder, id, currentScore, data_Ranking.mRrank, true, true));
		}

		char* bodyBuffer = reinterpret_cast<char*>(builder.GetBufferPointer());
		mNetworkManager.Send(socketId, contentsType, bodyBuffer, builder.GetSize());

		std::string log = "Send RESPONSE_PLAYER_RANKING - ID: " + playerId + " Score: " + std::to_string(currentScore);
		Utility::Debug("Business", "Hub", log);
	}
}