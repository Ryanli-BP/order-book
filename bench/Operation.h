#pragma once

#include "Side.h"
#include "Usings.h"

#include <vector>

enum class OpType {
    Add,      // order of these must match operationDist_ weights
    Cancel,
    Market
};

struct Operation {
    OpType type;
    OrderId id;
    Side side;
    Price price;
    Quantity quantity;
};

using Operations = std::vector<Operation>;
