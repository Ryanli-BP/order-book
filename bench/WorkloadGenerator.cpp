#include "WorkloadGenerator.h"

namespace {
    constexpr Price MidPrice = 10000;
    constexpr Price CrossTicks = 5;   // how far past the mid an order can land and cross
}

WorkloadGenerator::WorkloadGenerator(std::uint64_t seed)
    : rng_{ seed }
{
}

Operation WorkloadGenerator::GenerateAdd() {
    OrderId id = nextId_++;
    addedIds_.push_back(id);

    Side side = sideDist_(rng_) ? Side::Buy : Side::Sell;
    Quantity quantity = quantityDist_(rng_);
    int offset = offsetDist_(rng_);

    Price price{};
    if (side == Side::Buy) {
        price = MidPrice - offset + CrossTicks;
    }
    else {
        price = MidPrice + offset - CrossTicks;
    }

    return Operation{ OpType::Add, id, side, price, quantity };
}

Operation WorkloadGenerator::GenerateCancel() {
    std::uniform_int_distribution<std::size_t> indexDist(0, addedIds_.size() - 1);
    std::size_t index = indexDist(rng_);
    OrderId id = addedIds_[index];
    
    return Operation{ OpType::Cancel, id};

}

Operation WorkloadGenerator::GenerateMarket() {
    OrderId id = nextId_++;
    addedIds_.push_back(id);

    Side side = sideDist_(rng_) ? Side::Buy : Side::Sell;
    Quantity quantity = quantityDist_(rng_);
    
    return Operation{ OpType::Market, id, side, InvalidPrice, quantity};

}

Operations WorkloadGenerator::GenerateWorkload(std::size_t prefillCount, std::size_t operationCount) {
    Operations operations{};
    operations.reserve(prefillCount + operationCount);

    // prefill: adds only, so the book starts populated
    for (std::size_t i{0}; i < prefillCount; ++i) {
        operations.push_back(GenerateAdd());
    }

    for (std::size_t i{0}; i < operationCount; ++i) {

        switch(static_cast<OpType>(operationDist_(rng_))) {
            case OpType::Add:
                operations.push_back(GenerateAdd());
                break;
            case OpType::Cancel:
                operations.push_back(GenerateCancel());
                break;
            case OpType::Market:
                operations.push_back(GenerateMarket());
                break;
        }        
    }

    return operations;
}
