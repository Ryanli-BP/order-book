#pragma once

#include "Operation.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

class WorkloadGenerator {
public:
    explicit WorkloadGenerator(std::uint64_t seed);

    Operations GenerateWorkload(std::size_t PrefillCount, std::size_t OperationCount);

private:
    Operation GenerateAdd();
    Operation GenerateCancel();
    Operation GenerateMarket();

    std::mt19937_64 rng_;
    std::discrete_distribution<int> operationDist_{60, 30, 10};
    std::bernoulli_distribution sideDist_{0.5};
    std::uniform_int_distribution<int> offsetDist_{0, 55};
    std::uniform_int_distribution<Quantity> quantityDist_{1, 100};

    OrderId nextId_{1};
    std::vector<OrderId> addedIds_;
};
