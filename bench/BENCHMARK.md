operations: 1000000 (after 10000 prefill)
p50:    1042 ns
p99:    427988 ns
p99.9:  541870 ns
max:    17632107 ns
throughput: 0.01 M ops/s
trades: 255967 (latency pass) / 255967 (throughput pass)

//added operation type analysis
operations: 1000000 (after 10000 prefill)
p50:    1112 ns
p99:    426193 ns
p99.9:  563753 ns
max:    22604615 ns
throughput: 0.01 M ops/s
trades: 255967 (latency pass) / 255967 (throughput pass)
add      count   600420   p50    47088 ns   p99   445012 ns
cancel   count   299667   p50      411 ns   p99     1563 ns
market   count    99913   p50      642 ns   p99     7283 ns
final book size: 314656


// iterator = std::next(orders.begin(), orders.size() - 1); -> iterator = std::prev(askLevel.end());
operations: 1000000 (after 10000 prefill)
p50:    190 ns
p99:    2335 ns
p99.9:  10399 ns
max:    19414633 ns
throughput: 3.95 M ops/s
trades: 255967 (latency pass) / 255967 (throughput pass)
add      count   600420   p50      150 ns   p99     2445 ns
cancel   count   299667   p50      371 ns   p99      952 ns
market   count    99913   p50      341 ns   p99     2746 ns
final book size: 314656

//added spikes + reformatting, ran multiple times(deterministic behavior, same 3 indices show >1ms)
type         count       p50       p99     p99.9         max
all        1000000       160      2412      9896    16834136
add         600420       140      2502     12311     8925291
cancel      299667       230       890      1411       69349
market       99913       320      2792     14249    16834136
throughput: 4.23 M ops/s
spikes over 1ms: 3
  index   126387  add          1368425 ns
  index   266163  add          8925291 ns
  index   552150  market      16834136 ns
trades: 255967 / 255967   final book size: 314656

//suspecting rehash of unodered_map to be the culprit of, spike, proven with prints
rehash at index 18691: 10273 -> 20753 buckets, 10274 orders
rehash at index 56123: 20753 -> 42043 buckets, 20754 orders
rehash at index 126387: 42043 -> 85229 buckets, 42044 orders
rehash at index 266163: 85229 -> 172933 buckets, 85230 orders
rehash at index 552150: 172933 -> 351061 buckets, 172931 orders
type         count       p50       p99     p99.9         max
all        1000000       171      2359      5671    16852197
add         600420       141      2429      5872     6012989
cancel      299667       271       883      1275       67184
market       99913       321      2691      8291    16852197
throughput: 4.09 M ops/s
spikes over 1ms: 3
  index   126387  add          3142471 ns
  index   266163  add          6012989 ns
  index   552150  market      16852197 ns
trades: 255967 / 255967   final book size: 314656

//Constructor of Orderbook now reserve(), eliminating rehash of unordered_map during add operations
type         count       p50       p99     p99.9         max
all        1000000       121      2314     17402      116462
add         600420       110      2374     18555       92462
cancel      299667       140       881      1172      116462
market       99913       260      2675     19256       60012
throughput: 5.99 M ops/s
spikes over 1ms: none
trades: 255967 / 255967   final book size: 314656

//Added allocations/operations metric
ryanl@DESKTOP-G285NJN:~/order-book$ ./build-release/orderbook_bench
type         count       p50       p99     p99.9         max   allocs/op
all        1000000       106      2218     15271       80539        3.01
add         600420        97      2267     16978       80539        4.18
cancel      299667       126       840      1226       40336        0.00
market       99913       232      2518     17402       60286        5.00
throughput: 7.06 M ops/s
spikes over 1ms: none
trades: 255967 / 255967   final book size: 314656

//Removed "trades.reserve(orders_.size())"
type         count       p50       p99     p99.9         max   allocs/op
all        1000000        87      1102      5567      116496        2.56
add         600420        77      2291      8651      116496        3.28
cancel      299667       126       860      1227       50986        0.00
market       99913       222      2455      8023       94540        5.91
throughput: 8.35 M ops/s
spikes over 1ms: none
trades: 255967 / 255967   final book size: 314656

//trades is now a member variable of orderbook, removing the need of allocating a vector/MatchOrder() call
type         count       p50       p99     p99.9         max   allocs/op
all        1000000        87      1032      5148       65124        2.45
add         600420        77      2150      5455       65124        3.25
cancel      299667       116       800      1128       37531        0.00
market       99913       203      2304      9293       26788        5.00
throughput: 8.84 M ops/s
spikes over 1ms: none
trades: 255967 / 255967   final book size: 314656