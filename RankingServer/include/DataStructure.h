#pragma once  
#include<string> 

#include <nlohmann/json.hpp> 

#include "Converter.h"

namespace Business
{
    enum class TableType
    {
        Score,
        Ranking,
        Unknown
    };

    struct Data_Score
    {
        std::string playerId;
        int score;
        long long last_update;

        Data_Score(const nlohmann::json& jsonStr)
        {
            playerId = jsonStr["player_id"];
            score = jsonStr["score"];
            last_update = jsonStr["last_update"];
        }

        nlohmann::json static toJson(std::string id, int score, std::time_t last_update)
        {
            nlohmann::json rankingJson =
            {
                 {"player_id", id},
                 {"score", score},
                 {"last_update", last_update}
            };

            return rankingJson;
        }
    };
}