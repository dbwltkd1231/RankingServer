#pragma once
#include<iostream>
#include<string>
#include<functional>

#include "flatbuffers/flatbuffers.h"
#include "RANKING_PROTOCOL_generated.h"

#include "Log.h"

namespace Business
{
	class RankingSystem
	{
	public:
		RankingSystem();
		~RankingSystem();

		void Ready(std::function<void(uint32_t, uint32_t, char*)> sendCallback);
		void MessageRead(uint32_t socketId, uint32_t boydSize, uint32_t contentsType, char* bodyBuffer);

	private:
		std::function<void(uint32_t, uint32_t, char*)> mSendCallback;

	};
}