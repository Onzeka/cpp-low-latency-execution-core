

add_executable(orderbook_bench bench_matchingEngine.cpp)

target_link_libraries(orderbook_bench PRIVATE MatchingCore benchmark::benchmark benchmark::benchmark_main)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message(WARNING "Building benchmarks in Debug mode! Results will be useless.")
endif()

target_compile_options(orderbook_bench PRIVATE -O3 -march=native)