#pragma once

#include <map>
#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "Usings.h"
#include "Order.h"
#include "OrderModify.h"
#include "OrderbookLevelInfos.h"
#include "Trade.h"

class Orderbook {

private:

    struct OrderEntry {
        OrderPointer order_ { nullptr };
        OrderQueue::iterator location_; //for bids_/asks_
    };

    std::map<Price, OrderQueue, std::greater<Price>> bids_;
    std::map<Price, OrderQueue, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;

    bool CanMatch(Side side, Price price) const;

    bool CanFullyFill(Side side, Price price, Quantity quantity) const;

    Trades MatchOrders();

public:

    explicit Orderbook(std::size_t expectedOrders = 0) {
        orders_.reserve(expectedOrders);
    }

    Trades AddOrder(OrderPointer order);

    void CancelOrder(OrderId orderId);

    Trades ModifyOrder(OrderModify order);

    void EndOfDay();

    std::size_t Size() const { return orders_.size(); }

    std::size_t OrderLookupBucketCount() const { return orders_.bucket_count(); }

    bool HasOrder(OrderId orderId) const { return orders_.contains(orderId); }

    OrderbookLevelInfos GetOrderInfos() const;
    
};