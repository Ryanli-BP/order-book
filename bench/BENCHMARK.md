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