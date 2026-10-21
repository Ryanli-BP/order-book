#include <gtest/gtest.h>

#include "Orderbook.h"

namespace {
    const Trades& Buy(Orderbook& orderbook, OrderId id, Price price, Quantity quantity, OrderType orderType=OrderType::GoodTillCancel) {
        return orderbook.AddOrder(orderType, id, Side::Buy, price, quantity);
    }
    const Trades& Sell(Orderbook& orderbook, OrderId id, Price price, Quantity quantity, OrderType orderType=OrderType::GoodTillCancel) {
        return orderbook.AddOrder(orderType, id, Side::Sell, price, quantity);
    }
}

//DEFAULT ORDERTYPE = OrderType::GoodTillCancel

TEST(OrderbookTests, CrossingOrdersTrade) {
    Orderbook orderbook{};
    constexpr OrderId buyer = 1;
    constexpr OrderId seller = 2;

    Buy(orderbook, buyer, 100, 10);

    auto trades = Sell(orderbook, seller, 100, 4);

    //trades info is correct
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].GetBidTrade().orderId_, buyer);
    EXPECT_EQ(trades[0].GetAskTrade().orderId_, seller);
    EXPECT_EQ(trades[0].GetAskTrade().quantity_, 4u);

    //orderbook info is correct
    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetAsks().empty());
    ASSERT_EQ(infos.GetBids().size(), 1u);
    EXPECT_EQ(infos.GetBids()[0].price_, 100);
    EXPECT_EQ(infos.GetBids()[0].quantity_, 6u);
}

TEST(OrderbookTests, WalksMultipleLevels) {
    Orderbook orderbook{};
    constexpr OrderId cheapSeller = 1;
    constexpr OrderId expensiveSeller = 2;
    constexpr OrderId buyer = 3;

    Sell(orderbook, cheapSeller, 100, 5);
    Sell(orderbook, expensiveSeller, 101, 5);

    auto trades = Buy(orderbook, buyer, 101, 8);

    //best price first, then the next level
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].GetAskTrade().orderId_, cheapSeller);
    EXPECT_EQ(trades[0].GetAskTrade().quantity_, 5u);
    EXPECT_EQ(trades[1].GetAskTrade().orderId_, expensiveSeller);
    EXPECT_EQ(trades[1].GetAskTrade().quantity_, 3u);

    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetBids().empty());
    ASSERT_EQ(infos.GetAsks().size(), 1u);
    EXPECT_EQ(infos.GetAsks()[0].price_, 101);
    EXPECT_EQ(infos.GetAsks()[0].quantity_, 2u);
}

TEST(OrderbookTests, TimePriorityWithinLevel) {
    Orderbook orderbook{};
    constexpr OrderId firstSeller = 1;
    constexpr OrderId secondSeller = 2;
    constexpr OrderId buyer = 3;

    Sell(orderbook, firstSeller, 100, 5);
    Sell(orderbook, secondSeller, 100, 5);

    auto trades = Buy(orderbook, buyer, 100, 5);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].GetAskTrade().orderId_, firstSeller);

    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetBids().empty());
    ASSERT_EQ(infos.GetAsks().size(), 1u);
    EXPECT_EQ(infos.GetAsks()[0].quantity_, 5u);
}

TEST(OrderbookTests, FakCancelsRemainder) {
    Orderbook orderbook{};
    constexpr OrderId seller = 1;
    constexpr OrderId buyer = 2;

    Sell(orderbook, seller, 100, 5);

    auto trades = Buy(orderbook, buyer, 100, 8, OrderType::FillAndKill);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].GetBidTrade().quantity_, 5u);

    //the 3 left over must not rest
    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetBids().empty());
    EXPECT_TRUE(infos.GetAsks().empty());
}

TEST(OrderbookTests, FokHit) {
    Orderbook orderbook{};
    constexpr OrderId cheapSeller = 1;
    constexpr OrderId expensiveSeller = 2;
    constexpr OrderId buyer = 3;

    Sell(orderbook, cheapSeller, 100, 5);
    Sell(orderbook, expensiveSeller, 101, 5);

    auto trades = Buy(orderbook, buyer, 101, 10, OrderType::FillOrKill);

    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].GetAskTrade().orderId_, cheapSeller);
    EXPECT_EQ(trades[1].GetAskTrade().orderId_, expensiveSeller);

    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetBids().empty());
    EXPECT_TRUE(infos.GetAsks().empty());
}

TEST(OrderbookTests, FokMiss) {
    Orderbook orderbook{};
    constexpr OrderId cheapSeller = 1;
    constexpr OrderId expensiveSeller = 2;
    constexpr OrderId tooExpensiveSeller = 3;
    constexpr OrderId buyer = 4;

    Sell(orderbook, cheapSeller, 100, 5);
    Sell(orderbook, expensiveSeller, 101, 5);
    Sell(orderbook, tooExpensiveSeller, 102, 5); //enough in total, but above the limit

    auto trades = Buy(orderbook, buyer, 101, 11, OrderType::FillOrKill);

    EXPECT_TRUE(trades.empty());

    //book unchanged
    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetBids().empty());
    ASSERT_EQ(infos.GetAsks().size(), 3u);
    EXPECT_EQ(infos.GetAsks()[0].quantity_, 5u);
    EXPECT_EQ(infos.GetAsks()[1].quantity_, 5u);
    EXPECT_EQ(infos.GetAsks()[2].quantity_, 5u);
}

TEST(OrderbookTests, MarketWalksBook) {
    Orderbook orderbook{};
    constexpr OrderId cheapSeller = 1;
    constexpr OrderId expensiveSeller = 2;
    constexpr OrderId buyer = 3;

    Sell(orderbook, cheapSeller, 100, 5);
    Sell(orderbook, expensiveSeller, 101, 5);

    auto trades = orderbook.AddOrder(OrderType::Market, buyer, Side::Buy, InvalidPrice, 8);

    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].GetAskTrade().orderId_, cheapSeller);
    EXPECT_EQ(trades[0].GetAskTrade().quantity_, 5u);
    EXPECT_EQ(trades[1].GetAskTrade().orderId_, expensiveSeller);
    EXPECT_EQ(trades[1].GetAskTrade().quantity_, 3u);

    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetBids().empty());
    ASSERT_EQ(infos.GetAsks().size(), 1u);
    EXPECT_EQ(infos.GetAsks()[0].price_, 101);
    EXPECT_EQ(infos.GetAsks()[0].quantity_, 2u);
}

TEST(OrderbookTests, CancelRemovesOrder) {
    Orderbook orderbook{};
    constexpr OrderId cancelledBuyer = 1;
    constexpr OrderId otherBuyer = 2;

    Buy(orderbook, cancelledBuyer, 100, 10);
    Buy(orderbook, otherBuyer, 99, 5);

    orderbook.CancelOrder(cancelledBuyer);

    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetAsks().empty());
    ASSERT_EQ(infos.GetBids().size(), 1u);
    EXPECT_EQ(infos.GetBids()[0].price_, 99);
}

TEST(OrderbookTests, ModifyLosesTimePriority) {
    Orderbook orderbook{};
    constexpr OrderId modifiedSeller = 1;
    constexpr OrderId otherSeller = 2;
    constexpr OrderId buyer = 3;

    Sell(orderbook, modifiedSeller, 100, 5);
    Sell(orderbook, otherSeller, 100, 5);

    orderbook.ModifyOrder(OrderModify{ modifiedSeller, Side::Sell, 100, 3 }); //same price, back of the queue

    auto trades = Buy(orderbook, buyer, 100, 5);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].GetAskTrade().orderId_, otherSeller);

    auto infos = orderbook.GetOrderInfos();
    EXPECT_TRUE(infos.GetBids().empty());
    ASSERT_EQ(infos.GetAsks().size(), 1u);
    EXPECT_EQ(infos.GetAsks()[0].quantity_, 3u);
}

TEST(OrderbookTests, EndOfDayCancelsOnlyGfd) {
    Orderbook orderbook{};
    constexpr OrderId gfdBuyer = 1;
    constexpr OrderId gtcBuyer = 2;
    constexpr OrderId gfdSeller = 3;
    constexpr OrderId gtcSeller = 4;

    Buy(orderbook, gfdBuyer, 100, 5, OrderType::GoodForDay);
    Buy(orderbook, gtcBuyer, 99, 5);
    Sell(orderbook, gfdSeller, 105, 3, OrderType::GoodForDay);
    Sell(orderbook, gtcSeller, 106, 2);

    orderbook.EndOfDay();

    auto infos = orderbook.GetOrderInfos();
    ASSERT_EQ(infos.GetBids().size(), 1u);
    EXPECT_EQ(infos.GetBids()[0].price_, 99);
    ASSERT_EQ(infos.GetAsks().size(), 1u);
    EXPECT_EQ(infos.GetAsks()[0].price_, 106);
}
