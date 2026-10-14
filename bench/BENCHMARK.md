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

//suspecting rehash or unodered_map to be the culprit of, spike, proven with prints
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