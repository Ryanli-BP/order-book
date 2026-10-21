cmake_minimum_required(VERSION 3.24)
project(OrderBook CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

option(ENABLE_SANITIZERS "Build with ASan + UBSan" ON)
if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# The engine: every .cpp except main.cpp and the UI
add_library(orderbook_lib engine/Orderbook.cpp engine/OrderPool.cpp)
target_include_directories(orderbook_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine)

# The interactive UI
add_executable(orderbook ui/main.cpp ui/TerminalUI.cpp)
target_link_libraries(orderbook PRIVATE orderbook_lib)

add_executable(orderbook_bench
    bench/Benchmark.cpp
    bench/WorkloadGenerator.cpp
    bench/BenchmarkRunner.cpp
    bench/BenchmarkReport.cpp
    bench/AllocationCounter.cpp
)
target_link_libraries(orderbook_bench PRIVATE orderbook_lib)

# GoogleTest, downloaded and built by CMake
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# The tests
enable_testing()
add_executable(orderbook_tests tests/OrderbookTests.cpp)
target_link_libraries(orderbook_tests PRIVATE orderbook_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(orderbook_tests)
