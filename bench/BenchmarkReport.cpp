#include "BenchmarkReport.h"

#include <chrono>
#include <cstddef>
#include <format>
#include <iostream>
#include <string_view>
#include <vector>

namespace {
    using std::chrono::nanoseconds;

    // Expects latencies already sorted. p is a fraction, e.g. 0.99 for p99.
    nanoseconds Percentile(const std::vector<nanoseconds>& sorted, double p) {
        std::size_t index = static_cast<std::size_t>(p * static_cast<double>(sorted.size()));
        if (index >= sorted.size()) {
            index = sorted.size() - 1;
        }
        return sorted[index];
    }
}

std::string_view OpTypeName(OpType type) {
    switch (type) {
        case OpType::Add:
            return "add";
        case OpType::Cancel:
            return "cancel";
        case OpType::Market:
            return "market";
    }
    return "unknown";
}

void PrintLatencyHeader() {
    std::cout << std::format("{:<8}{:>10}{:>10}{:>10}{:>10}{:>12}{:>12}\n",
                            "type", "count", "p50", "p99", "p99.9", "max", "allocs/op");
}

void PrintLatencyRow(std::string_view label, const std::vector<nanoseconds>& sorted, std::size_t allocations) {
    const double allocationsPerOp = static_cast<double>(allocations) / static_cast<double>(sorted.size());

    std::cout << std::format("{:<8}{:>10}{:>10}{:>10}{:>10}{:>12}{:>12.2f}\n",
                             label,
                             sorted.size(),
                             Percentile(sorted, 0.50).count(),
                             Percentile(sorted, 0.99).count(),
                             Percentile(sorted, 0.999).count(),
                             sorted.back().count(),
                             allocationsPerOp);
}

void PrintLatencyTable(const LatencyResult& latency) {
    std::size_t totalAllocations = 0;
    for (std::size_t allocations : latency.allocationsByType) {
        totalAllocations += allocations;
    }

    PrintLatencyHeader();
    PrintLatencyRow("all", latency.latencies, totalAllocations);

    for (std::size_t i{0}; i < latency.latenciesByType.size(); ++i) {
        PrintLatencyRow(OpTypeName(static_cast<OpType>(i)), latency.latenciesByType[i], latency.allocationsByType[i]);
    }
}

void PrintThroughput(const ThroughputResult& throughput, std::size_t operationCount) {
    const double operationsPerSecond = static_cast<double>(operationCount) / throughput.elapsed.count();
    std::cout << std::format("throughput: {:.2f} M ops/s\n", operationsPerSecond / 1'000'000.0);
}

void PrintSpikes(const std::vector<Spike>& spikes) {
    if (spikes.empty()) {
        std::cout << "spikes over 1ms: none\n";
        return;
    }

    std::cout << std::format("spikes over 1ms: {}\n", spikes.size());
    for (const Spike& spike : spikes) {
        std::cout << std::format("  index {:>8}  {:<8}{:>12} ns\n",
                                spike.index,
                                OpTypeName(spike.type),
                                spike.latency.count());
    }
}
