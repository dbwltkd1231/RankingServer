#pragma once
#include "RankingSystem.h"
#include "DatabaseWorker.h"

namespace Business
{

	RankingSystem::RankingSystem()
	{

	}

	RankingSystem::~RankingSystem()
	{

	}

	void RankingSystem::Ready(std::function<void(uint32_t, uint32_t, char*)> sendCallback)
	{
		mSendCallback = sendCallback;
	}

	void RankingSystem::MessageRead(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer)// ntohl�Ȱɷ� ���޹޾ƾ���.
	{
		mSendCallback(socketId, contentsType, bodyBuffer);
	}
}