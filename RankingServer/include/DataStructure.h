#pragma once  
#include <string>

#include <nlohmann/json.hpp> 

namespace Business
{
    struct Data_Score
    {
        std::string mPlayerId;
        int mScore;
        long long mLastUpdate;

        Data_Score(const nlohmann::json& jsonStr)
        {
            mPlayerId = jsonStr["player_id"];
            mScore = jsonStr["score"];
            mLastUpdate = jsonStr["last_update"];
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
        int mRrank;
        std::string mPlayerId;
        int mScore;

        Data_Ranking(const nlohmann::json& jsonStr)
        {
            mRrank = jsonStr["rank"];
            mPlayerId = jsonStr["player_id"];
            mScore = jsonStr["score"];
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