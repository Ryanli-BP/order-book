#include "BenchmarkReport.h"
#include "BenchmarkRunner.h"
#include "WorkloadGenerator.h"

#include <cstddef>
#include <format>
#include <iostream>

namespace {
    constexpr std::size_t OperationCount = 1'000'000;
    constexpr std::size_t PrefillCount = OperationCount / 100;
}

int main() {
    WorkloadGenerator generator{ 42 };
    const Operations operations = generator.GenerateWorkload(PrefillCount, OperationCount);

    const LatencyResult latency = RunLatencyPass(operations, PrefillCount, OperationCount);
    const ThroughputResult throughput = RunThroughputPass(operations, PrefillCount, OperationCount);

    PrintLatencyTable(latency);
    PrintThroughput(throughput, OperationCount);
    PrintSpikes(latency.spikes);

    std::cout << std::format("trades: {} / {}   final book size: {}\n",
                             latency.tradeCount, throughput.tradeCount, latency.finalBookSize);
}
