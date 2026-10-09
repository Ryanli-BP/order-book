#include "Orderbook.h"
#include "WorkloadGenerator.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <iostream>
#include <memory>
#include <vector>

namespace {
    constexpr std::size_t OperationCount = 1'000'000;
    constexpr std::size_t PrefillCount = OperationCount / 100;

    using Clock = std::chrono::steady_clock;
    using std::chrono::nanoseconds;
    using std::chrono::duration_cast;

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

    // The first PrefillCount operations only set up the book, so they're never timed.
    void ApplyPrefill(Orderbook& orderbook, const Operations& operations) {
        for (std::size_t i{0}; i < PrefillCount; ++i) {
            Apply(orderbook, operations[i]);
        }
    }

    struct LatencyResult {
        std::vector<nanoseconds> latencies;
        std::array<std::vector<nanoseconds>, 3> latenciesByType;   // indexed by OpType: add, cancel, market
        std::size_t tradeCount;
        std::size_t finalBookSize;
    };

    // Times every operation individually.
    LatencyResult RunLatencyPass(const Operations& operations) {
        Orderbook orderbook{};
        ApplyPrefill(orderbook, operations);

        LatencyResult result{};
        result.latencies.reserve(OperationCount);

        for (std::size_t i{PrefillCount}; i < operations.size(); ++i) {
            Clock::time_point before = Clock::now();
            result.tradeCount += Apply(orderbook, operations[i]);
            Clock::time_point after = Clock::now();

            nanoseconds latency = duration_cast<nanoseconds>(after - before);
            result.latencies.push_back(latency);

            std::size_t typeIndex = static_cast<std::size_t>(operations[i].type);
            result.latenciesByType[typeIndex].push_back(latency);
        }

        result.finalBookSize = orderbook.Size();
        return result;
    }

    struct ThroughputResult {
        std::chrono::duration<double> elapsed;
        std::size_t tradeCount;
    };

    // One timer around the whole run, so per-operation clock calls don't distort it.
    ThroughputResult RunThroughputPass(const Operations& operations) {
        Orderbook orderbook{};
        ApplyPrefill(orderbook, operations);

        ThroughputResult result{};

        Clock::time_point before = Clock::now();
        for (std::size_t i{PrefillCount}; i < operations.size(); ++i) {
            result.tradeCount += Apply(orderbook, operations[i]);
        }
        Clock::time_point after = Clock::now();

        result.elapsed = after - before;
        return result;
    }

    // Expects latencies already sorted. p is a fraction, e.g. 0.99 for p99.
    nanoseconds Percentile(const std::vector<nanoseconds>& sorted, double p) {
        std::size_t index = static_cast<std::size_t>(p * static_cast<double>(sorted.size()));
        if (index >= sorted.size()) {
            index = sorted.size() - 1;
        }
        return sorted[index];
    }
}

int main() {
    WorkloadGenerator generator{ 42 };
    const Operations operations = generator.GenerateWorkload(PrefillCount, OperationCount);

    LatencyResult latency = RunLatencyPass(operations);
    ThroughputResult throughput = RunThroughputPass(operations);

    std::sort(latency.latencies.begin(), latency.latencies.end());

    const double operationsPerSecond = static_cast<double>(OperationCount) / throughput.elapsed.count();

    std::cout << std::format("operations: {} (after {} prefill)\n", OperationCount, PrefillCount);
    std::cout << std::format("p50:    {} ns\n", Percentile(latency.latencies, 0.50).count());
    std::cout << std::format("p99:    {} ns\n", Percentile(latency.latencies, 0.99).count());
    std::cout << std::format("p99.9:  {} ns\n", Percentile(latency.latencies, 0.999).count());
    std::cout << std::format("max:    {} ns\n", latency.latencies.back().count());
    std::cout << std::format("throughput: {:.2f} M ops/s\n", operationsPerSecond / 1'000'000.0);
    std::cout << std::format("trades: {} (latency pass) / {} (throughput pass)\n",
                             latency.tradeCount, throughput.tradeCount);

    constexpr std::array<const char*, 3> TypeNames{ "add", "cancel", "market" };   // same order as OpType

    for (std::size_t type{0}; type < 3; ++type) {
        std::vector<nanoseconds>& typeLatencies = latency.latenciesByType[type];
        std::sort(typeLatencies.begin(), typeLatencies.end());

        std::cout << std::format("{:<8} count {:>8}   p50 {:>8} ns   p99 {:>8} ns\n",
                                TypeNames[type],
                                typeLatencies.size(),
                                Percentile(typeLatencies, 0.50).count(),
                                Percentile(typeLatencies, 0.99).count());
    }

    std::cout << std::format("final book size: {}\n", latency.finalBookSize);
}