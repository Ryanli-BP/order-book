#pragma once

#include "Operation.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <vector>

struct Spike {
    std::size_t index;
    OpType type;
    std::chrono::nanoseconds latency;
};

struct LatencyResult {
    std::vector<std::chrono::nanoseconds> latencies;
    std::array<std::vector<std::chrono::nanoseconds>, OpTypeCount> latenciesByType;   // indexed by OpType: add, cancel, market
    std::size_t tradeCount;
    std::size_t finalBookSize;
    std::vector<Spike> spikes{};
};

struct ThroughputResult {
    std::chrono::duration<double> elapsed;
    std::size_t tradeCount;
};

// Times every operation individually.
LatencyResult RunLatencyPass(const Operations& operations, std::size_t prefillCount);

// One timer around the whole run, so per-operation clock calls don't distort it.
ThroughputResult RunThroughputPass(const Operations& operations, std::size_t prefillCount);
