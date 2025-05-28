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
		databaseWorker.Initalize();
		databaseWorker.RedisConnect(ip, redisPort);
		databaseWorker.DataLoadInSQL();

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

		networkManager.Initialize(port, socketMax);
		networkManager.Ready(mReceiveCallback);
		mRankingSystem.Ready(mSendCallback);
	}

	void Hub::Read(uint32_t socketId, uint32_t bodySize, uint32_t contentsType, char* bodyBuffer)
	{
		mRankingSystem.MessageRead(socketId, bodySize, contentsType, bodyBuffer);
	}

	void Hub::Send(uint32_t socketId, uint32_t requestType, char* bodyBuffer)
	{
		auto messageType = static_cast<protocol::MessageContent>(requestType);

		switch (messageType)
		{
		case protocol::MessageContent_REQUEST_PLAYER_RANKING:
		{
			Utility::Debug("Business", "Hub", "Read : REQUEST_PLAYER_RANKING");

			auto message_wrapper = flatbuffers::GetRoot<protocol::REQUEST_PLAYER_RANKING>(bodyBuffer); // FlatBuffers 버퍼에서 루트 객체 가져오기
			Response_PlayerRanking(socketId, message_wrapper->player_id()->str());
			break;
		}
		case protocol::MessageContent_REQUEST_SAVE_SCORE:
		{
			Utility::Debug("Business", "Hub", "Read : REQUEST_SAVE_SCORE");

			auto  REQUEST_SAVE_SCORE = flatbuffers::GetRoot<protocol::REQUEST_SAVE_SCORE>(bodyBuffer);
			std::time_t timeTValue = static_cast<std::time_t>(REQUEST_SAVE_SCORE->last_update()); // 변환
			Response_SaveScore(socketId, REQUEST_SAVE_SCORE->player_id()->str(), REQUEST_SAVE_SCORE->score(), timeTValue);
			break;
		}
		default:
		{
			Utility::Debug("Business", "Hub", "Read : CONTENTS TYPE ERROR !!");
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

			databaseWorker.ScoreDataSave();
			databaseWorker.RankingUpdate();
			databaseWorker.RankingDataLoad();
		}
	}

	void Hub::Response_SaveScore(uint32_t socketId, std::string playerID, int newScore, std::time_t unixUpdateDate)
	{
		auto scoreJson = Data_Score::toJson(playerID, newScore, unixUpdateDate);
		std::string jsonString = scoreJson.dump();

		databaseWorker.SetCachedData("Score", playerID, jsonString, 60);

		flatbuffers::FlatBufferBuilder builder;
		auto response_contents_type = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_SAVE_SCORE);
		builder.Finish(protocol::CreateRESPONSE_SAVE_SCORE(builder, true));
		char* bodyBuffer = reinterpret_cast<char*>(builder.GetBufferPointer());
		networkManager.Send(socketId, response_contents_type, bodyBuffer, builder.GetSize());

		std::string log = "Send RESPONSE_SAVE_SCORE : Success ";
		Utility::Debug("Business", "Hub", log);
	}

	void Hub::Response_PlayerRanking(uint32_t socketId, std::string playerID)
	{
		auto response_contents_type = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_PLAYER_RANKING);
		auto playerRanking = databaseWorker.GetCachedData("Ranking", playerID);
		auto playerScore = databaseWorker.GetCachedData("Score", playerID);

		flatbuffers::FlatBufferBuilder builder;
		auto id = builder.CreateString(playerID);
		int score = 0;

		if (playerScore == "")
		{
			builder.Finish(protocol::CreateRESPONSE_PLAYER_RANKING(builder, id, 0, 0, false, false));
			return;
		}

		Business::Data_Score data_Score(playerScore);
		score = data_Score.score;

		if (playerRanking == "")
		{
			builder.Finish(protocol::CreateRESPONSE_PLAYER_RANKING(builder, id, score, 0, false, true));
		}
		else
		{
			Business::Data_Ranking data_Ranking(playerRanking);
			builder.Finish(protocol::CreateRESPONSE_PLAYER_RANKING(builder, id, score, data_Ranking.rank, true, true));
		}

		char* bodyBuffer = reinterpret_cast<char*>(builder.GetBufferPointer());
		networkManager.Send(socketId, response_contents_type, bodyBuffer, builder.GetSize());

		std::string log = "Send RESPONSE_PLAYER_RANKING - ID: " + playerID + " Score: " + std::to_string(score);
		Utility::Debug("Business", "Hub", log);
	}
}
