#include "BenchmarkRunner.h"

#include "Orderbook.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

namespace {
    using Clock = std::chrono::steady_clock;
    using std::chrono::nanoseconds;
    using std::chrono::duration_cast;
    using namespace std::chrono_literals;

    // Performs one operation on the book. Returns how many trades it produced.
    std::size_t Apply(Orderbook& orderbook, const Operation& operation) {
        switch (operation.type) {
            case OpType::Add: {
                OrderPointer order = std::make_shared<Order>(
                    OrderType::GoodTillCancel,
                    operation.id,
                    operation.side,
                    operation.price,
                    operation.quantity);
                Trades trades = orderbook.AddOrder(order);
                return trades.size();
            }
            case OpType::Cancel: {
                orderbook.CancelOrder(operation.id);
                return 0;
            }
            case OpType::Market: {
                OrderPointer order = std::make_shared<Order>(
                    operation.id,
                    operation.side,
                    operation.quantity);
                Trades trades = orderbook.AddOrder(order);
                return trades.size();
            }
        }
        return 0;
    }

    // The first prefillCount operations only set up the book, so they're never timed.
    void ApplyPrefill(Orderbook& orderbook, const Operations& operations, std::size_t prefillCount) {
        for (std::size_t i{0}; i < prefillCount; ++i) {
            Apply(orderbook, operations[i]);
        }
    }
}

LatencyResult RunLatencyPass(const Operations& operations, std::size_t prefillCount) {
    Orderbook orderbook{};
    ApplyPrefill(orderbook, operations, prefillCount);

    LatencyResult result{};
    result.latencies.reserve(operations.size() - prefillCount);

    for (std::size_t i{prefillCount}; i < operations.size(); ++i) {
        Clock::time_point before = Clock::now();
        result.tradeCount += Apply(orderbook, operations[i]);
        Clock::time_point after = Clock::now();

        nanoseconds latency = duration_cast<nanoseconds>(after - before);
        result.latencies.push_back(latency);

        std::size_t typeIndex = static_cast<std::size_t>(operations[i].type);
        result.latenciesByType[typeIndex].push_back(latency);

        if (latency > 1ms) {
            result.spikes.push_back(Spike{i, operations[i].type, latency});
        }
    }

    std::sort(result.latencies.begin(), result.latencies.end());

    for (std::vector<nanoseconds>& typeLatencies : result.latenciesByType) {
        std::sort(typeLatencies.begin(), typeLatencies.end());
    }

    result.finalBookSize = orderbook.Size();
    return result;
}

ThroughputResult RunThroughputPass(const Operations& operations, std::size_t prefillCount) {
    Orderbook orderbook{};
    ApplyPrefill(orderbook, operations, prefillCount);

    ThroughputResult result{};

    Clock::time_point before = Clock::now();
    for (std::size_t i{prefillCount}; i < operations.size(); ++i) {
        result.tradeCount += Apply(orderbook, operations[i]);
    }
    Clock::time_point after = Clock::now();

    result.elapsed = after - before;
    return result;
}
