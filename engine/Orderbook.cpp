#include "Orderbook.h"
#include <numeric>

bool Orderbook::CanMatch(Side side, Price price) const {

    if (side == Side::Buy) {

        if (asks_.empty())
            return false;
    
        const auto& [bestAsk, _] = *asks_.begin();
        return price >= bestAsk;
    }
    else {
        if (bids_.empty())
            return false;

        const auto& [bestBid, _] = *bids_.begin();
        return price <= bestBid;
    }
}

bool Orderbook::CanFullyFill(Side side, Price price, Quantity quantity) const {
    Quantity availableQuantity{};

    if (side == Side::Buy) {
        for(const auto& [levelPrice, level] : asks_) {
            if (levelPrice > price) {
                break;
            }
            for (const auto& order : level) {
                availableQuantity += order->GetRemainingQuantity();
            }
        }
    }
    else {
        for(const auto& [levelPrice, level] : bids_) { 
            if (levelPrice < price) {
                break;
            }
            for (const auto& order : level) { 
                availableQuantity += order->GetRemainingQuantity(); 
            } 
        }
    }

    return availableQuantity >= quantity;
}

Trades Orderbook::AddOrder(OrderPointer order) {
    if (orders_.contains(order->GetOrderId()))
        return {};

    if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
        return {};

    if (order->GetOrderType() == OrderType::FillOrKill && !CanFullyFill(order->GetSide(), order->GetPrice(), order->GetInitialQuantity()))
        return {};

    if (order->GetOrderType() == OrderType::Market) {
        if (order->GetSide() == Side::Buy) {
            if (!asks_.size())
                return {};

            const auto& [price, _] = *asks_.rbegin();
            order->MarketToLimitOrder(price);
        }
        else {
            if (!bids_.size())
                return {};

            const auto& [price, _] = *bids_.rbegin();
            order->MarketToLimitOrder(price);
        }
    }
    
    OrderQueue::iterator iterator;

    if (order->GetSide() == Side::Buy) {
        auto& bidLevel = bids_[order->GetPrice()];
        bidLevel.push_back(order);
        iterator = std::prev(bidLevel.end());
    }
    else {
        auto& askLevel = asks_[order->GetPrice()];
        askLevel.push_back(order);
        iterator = std::prev(askLevel.end());
    }

    orders_.insert({ order->GetOrderId(), OrderEntry{order, iterator} });
    return MatchOrders();
}

Trades Orderbook::MatchOrders() {
    Trades trades;
    trades.reserve(orders_.size());

    while (true) {
        if (bids_.empty() || asks_.empty())
            break;

        auto& [bidPrice, bidLevel] = *bids_.begin();
        auto& [askPrice, askLevel] = *asks_.begin();

        if (bidPrice < askPrice)
            break;

        while (bidLevel.size() && askLevel.size()) {
            auto bid =  bidLevel.front();
            auto ask = askLevel.front();

            Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

            bid->Fill(quantity);
            ask->Fill(quantity);

            if (bid->isFilled()) {
                orders_.erase(bid->GetOrderId());
                bidLevel.pop_front();

            }
            if (ask->isFilled()) {
                askLevel.pop_front();
                orders_.erase(ask->GetOrderId());
            }
            
            trades.push_back(Trade{
                TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity },
                TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity } 
            });
        }

        if (bidLevel.empty()) {
            bids_.erase(bidPrice);
        }
        if (askLevel.empty()) {
            asks_.erase(askPrice);
        }
    }

    if (!bids_.empty()) {
        auto& [_, bids] = *bids_.begin();
        auto& order = bids.front();
        if (order->GetOrderType() == OrderType::FillAndKill)
            CancelOrder(order->GetOrderId());
    }
    if (!asks_.empty()) {
        auto& [_, asks] = *asks_.begin();
        auto& order = asks.front();
        if (order->GetOrderType() == OrderType::FillAndKill)
            CancelOrder(order->GetOrderId());
    }

    return trades;
}

void Orderbook::CancelOrder(OrderId orderId) {
    if (!orders_.contains(orderId))
        return;
    
    const auto& [order, orderIterator] = orders_.at(orderId);

    if (order->GetSide() == Side::Sell) {
        auto price = order->GetPrice();
        auto& askLevel = asks_.at(price);
        askLevel.erase(orderIterator);
        if (askLevel.empty()) {
            asks_.erase(price);
        }
    }
    else {
        auto price = order->GetPrice();
        auto& bidLevel = bids_.at(price);
        bidLevel.erase(orderIterator);
        if (bidLevel.empty()) {
            bids_.erase(price);
        }
    }

    orders_.erase(orderId);
}

Trades Orderbook::ModifyOrder(OrderModify order) {
    if (!orders_.contains(order.GetOrderId()))
        return {};
    
    const auto type = orders_.at(order.GetOrderId()).order_->GetOrderType(); //copy before CancelOrder erases the entry
    CancelOrder(order.GetOrderId());
    return AddOrder(order.ToOrderPointer(type));
}

OrderbookLevelInfos Orderbook::GetOrderInfos() const {
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(orders_.size());
    askInfos.reserve(orders_.size());

    auto CreateLevelInfos = [](Price price, const OrderQueue& orders) {
        const auto quantity = std::accumulate(orders.begin(), orders.end(), Quantity{0},
            [](Quantity runningSum, const OrderPointer& order) {
                return runningSum + order->GetRemainingQuantity();
            });
        return LevelInfo{ price, quantity };
    };

    for (const auto& [price, orders] : bids_)
        bidInfos.push_back(CreateLevelInfos(price, orders));
    
    for (const auto& [price, orders] : asks_)
        askInfos.push_back(CreateLevelInfos(price, orders));
    
    return OrderbookLevelInfos{ bidInfos, askInfos };
}

void Orderbook::EndOfDay() {
    OrderIds goodForDayOrders{};
    for (const auto& [orderId, orderEntry] : orders_) {
        if (orderEntry.order_->GetOrderType() == OrderType::GoodForDay)
            goodForDayOrders.push_back(orderId);
    }

    for (auto orderId : goodForDayOrders) {
        CancelOrder(orderId);
    }
}
