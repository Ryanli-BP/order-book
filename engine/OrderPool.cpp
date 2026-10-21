#include "OrderPool.h"
#include <stdexcept>

Order* OrderPool::Acquire(OrderType type, OrderId id, Side side, Price price, Quantity quantity) {
    if (freeIndices_.empty()) {
        throw std::runtime_error("Pool is Full, Cannot Acquire");
    }

    std::size_t index = freeIndices_.back();
    freeIndices_.pop_back();

    slots_[index] = Order(type, id, side, price, quantity);
    return &slots_[index];
}

void OrderPool::Release(Order* order) {
    std::size_t index = order - slots_.data();
    freeIndices_.push_back(index);
}