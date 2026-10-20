#pragma once

#include "BenchmarkRunner.h"
#include "Operation.h"

#include <chrono>
#include <cstddef>
#include <string_view>
#include <vector>

std::string_view OpTypeName(OpType type);

void PrintLatencyHeader();
void PrintLatencyRow(std::string_view label, const std::vector<std::chrono::nanoseconds>& sorted);
void PrintLatencyTable(const LatencyResult& latency);
void PrintThroughput(const ThroughputResult& throughput, std::size_t operationCount);
void PrintSpikes(const std::vector<Spike>& spikes);
