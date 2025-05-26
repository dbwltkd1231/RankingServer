#pragma once
#include "RankingServer.h"
#include "DatabaseWorker.h"

namespace Business
{

	RankingServer::RankingServer()
	{

	}

	RankingServer::~RankingServer()
	{

	}

	void RankingServer::MessageRead(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer)// ntohl된걸로 전달받아야함.
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);

		switch (messageType)
		{
		case protocol::MessageContent_REQUEST_PLAYER_RANKING:
		{
			Utility::Debug("Business", "RankingServer", "Read : REQUEST_PLAYER_RANKING");

			auto REQUEST_PLAYER_RANKING = flatbuffers::GetRoot<protocol::REQUEST_PLAYER_RANKING>(bodyBuffer);
			break;
		}
		case protocol::MessageContent_RESPONSE_PLAYER_RANKING:
		{
			Utility::Debug("Business", "RankingServer", "Read : RESPONSE_PLAYER_RANKING");

			auto RESPONSE_PLAYER_RANKING = flatbuffers::GetRoot<protocol::RESPONSE_PLAYER_RANKING>(bodyBuffer);
			break;
		}
		case protocol::MessageContent_REQUEST_SAVE_SCORE:
		{
			Utility::Debug("Business", "RankingServer", "Read : REQUEST_SAVE_SCORE");

			auto  REQUEST_SAVE_SCORE = flatbuffers::GetRoot<protocol::REQUEST_SAVE_SCORE>(bodyBuffer);
			break;
		}
		case protocol::MessageContent_RESPONSE_SABE_SCORE:
		{
			Utility::Debug("Business", "RankingServer", "Read : RESPONSE_SABE_SCORE");

			auto  RESPONSE_SABE_SCORE = flatbuffers::GetRoot<protocol::RESPONSE_SABE_SCORE>(bodyBuffer);
			break;
		}
		default:
		{
			Utility::Debug("Business", "RankingServer", "Read : CONTENTS TYPE ERROR !!");
			break;
		}
		}
	}
}