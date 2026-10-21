# order-book

Limit order book in C++23 with price-time priority matching.

Order types: GoodTillCancel, FillAndKill, FillOrKill, Market, GoodForDay.
Supports add, cancel, modify and a level 2 snapshot.

Started from the Coding Jesus orderbook tutorial. Since then I fixed three use-after-free bugs in it (found with ASan), implemented FillOrKill and GoodForDay my own way, and added the tests, terminal UI, benchmark and the performance work below.

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

- `bids_` / `asks_`: `std::map<Price, std::list<Order*>>`, one FIFO queue per price level
- `orders_`: `std::unordered_map<OrderId, {order, iterator into its queue}>`, so cancel is O(1) after the lookup; capacity reserved up front
- Orders live in a fixed-size object pool, so adding an order doesn't allocate it on the heap
- Matching reuses one trades buffer instead of allocating a new vector per call
- Prices are integer ticks

## Benchmark

10k resting orders, then 1M operations (60% limit adds, 30% cancels, 10% market orders), fixed seed.
Release build, GCC 13.3, WSL2. allocs/op = heap allocations per operation, counted by replacing the global `operator new`.

| Version | Throughput | p50 | p99 | allocs/op |
|---|---|---|---|---|
| Baseline | 0.01 M ops/s | 1112 ns | 426 µs | – |
| O(1) insert position | 3.95 M ops/s | 190 ns | 2.3 µs | – |
| Reserve order lookup table | 5.96 M ops/s | 120 ns | 2.3 µs | 3.01 |
| No per-call trades reserve | 8.35 M ops/s | 87 ns | 1.1 µs | 2.56 |
| Order object pool | 10.64 M ops/s | 68 ns | 646 ns | 1.61 |

- **O(1) insert position:** the baseline found a new order's queue position with `std::next(begin, size - 1)`, which walks the whole list. Switched to `std::prev(end)`.
- **Reserve order lookup table:** `orders_` was rehashing as the book grew, causing 1–17 ms latency spikes at fixed points. Reserving capacity up front removed them (max latency ~17 ms → ~0.1 ms).
- **No per-call trades reserve:** matching reserved a large trades buffer on every call, even though most adds produce no trades.
- **Order object pool:** replaced one `make_shared<Order>` per order with slots in a pre-allocated pool.