cmake_minimum_required(VERSION 3.15)
project(MatchingEngine VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -Wall -Werror)
endif()

enable_testing()

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)
find_package(pybind11 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmarks)