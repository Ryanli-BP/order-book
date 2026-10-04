#pragma once

enum class OrderType {
    GoodTillCancel,
    GoodForDay,
    FillAndKill,
    FillOrKill,
    Market
};
