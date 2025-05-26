#include "Hub.h"

namespace Business
{
	Hub::Hub()
	{

	}

	Hub::~Hub()
	{

	}

	void Hub::Initialize(std::string ip, int port, u_short redisPort, int socketMax, int sessionQueueMax)
	{
		databaseWorker.Initalize();
		databaseWorker.RedisConnect(ip, redisPort);
		databaseWorker.DataLoadInSQL();

		mReceiveCallback = std::function<void(uint32_t, uint32_t, uint32_t, char*)>(
			std::bind(&Business::RankingServer::MessageRead, std::ref(rankingServer),
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);


		networkManager.Initialize(port, socketMax);
		networkManager.Ready(sessionQueueMax, mReceiveCallback);

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

	void Hub::Response_SaveScore(std::string playerID, int score, long last_update)
	{
	
	}

	void Hub::Response_PlayerRanking(std::string playerID)
	{
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
			std::string dataCheck = "player_id : " + data_Ranking.playerId + " rank : " + std::to_string(data_Ranking.rank);
			Utility::Debug("RankingServer", "Main", dataCheck);
		}
	}
}
