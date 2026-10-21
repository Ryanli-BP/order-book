#pragma once

#include "Order.h"

#include <cstddef>
#include <numeric>
#include <vector>

class OrderPool {
public:
    explicit OrderPool(std::size_t capacity)
        : slots_(capacity)
        , freeIndices_(capacity) 
{
    std::iota(freeIndices_.begin(), freeIndices_.end(), 0);
}

    Order* Acquire(OrderType type, OrderId id, Side side, Price price, Quantity quantity);
    void Release(Order* order);

private:
    std::vector<Order> slots_;
    std::vector<std::size_t> freeIndices_;
};