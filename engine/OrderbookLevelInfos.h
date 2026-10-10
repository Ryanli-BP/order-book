#pragma once

#include "LevelInfo.h"
#include <vector>

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos { //level 2 (market depth)
public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks) 
        : bids_{ bids }
        , asks_{ asks }
    {
    }

    const LevelInfos& GetBids() const { return bids_; }
    const LevelInfos& GetAsks() const { return asks_; }


private:
    LevelInfos bids_;
    LevelInfos asks_;
};