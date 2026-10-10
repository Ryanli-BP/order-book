# order-book

Limit order book in C++23 with price-time priority matching.

Order types: GoodTillCancel, FillAndKill, FillOrKill, Market, GoodForDay.
Supports add, cancel, modify and a level 2 snapshot.

Started from the Coding Jesus orderbook tutorial. Since then I fixed three use-after-free bugs in it (found with ASan), implemented FillOrKill and GoodForDay my own way, and added the tests, terminal UI and benchmark.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug          # ASan + UBSan
cmake --build build
./build/orderbook_tests
./build/orderbook                                     # interactive UI

cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
cmake --build build-release
./build-release/orderbook_bench
```

## Design

- `bids_` / `asks_`: `std::map<Price, std::list<OrderPointer>>`, one FIFO queue per price level
- `orders_`: `std::unordered_map<OrderId, {order, iterator into its queue}>`, so cancel is O(1) after the lookup
- Prices are integer ticks

## Benchmark

10k resting orders, then 1M operations (60% limit adds, 30% cancels, 10% market orders), fixed seed.
Release build, GCC 13.3, WSL2.

| Version | Throughput | p50 | p99 |
|---|---|---|---|
| Baseline | 0.01 M ops/s | 1112 ns | 426 µs |
| O(1) insert position | 3.95 M ops/s | 190 ns | 2.3 µs |

The baseline found an insert's queue position with `std::next(begin, size - 1)`, which walks the whole list. Switched to `std::prev(end)`.
