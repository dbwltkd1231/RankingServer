#pragma once  
#include <string>

#include <nlohmann/json.hpp> 

namespace Business
{
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
            nlohmann::json scoreJson =
            {
                 {"player_id", id},
                 {"score", score},
                 {"last_update", last_update}
            };

            return scoreJson;
        }
    };

    struct Data_Ranking
    {
        int rank;
        std::string playerId;
        int score;

        Data_Ranking(const nlohmann::json& jsonStr)
        {
            rank = jsonStr["rank"];
            playerId = jsonStr["player_id"];
            score = jsonStr["score"];
        }

        nlohmann::json static toJson(int rank, std::string id, int score)
        {
            nlohmann::json rankingJson =
            {
                 {"rank", rank},
                 {"player_id", id},
                 {"score", score}
            };

            return rankingJson;
        }
    };
}