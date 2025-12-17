# High-Performance Limit Order Book & Matching Engine

## Overview
This project is a high-performance, single-threaded Limit Order Book (LOB) and Matching Engine implemented in **C++20**. 

It was designed to demonstrate low-latency coding techniques essential for high-frequency trading (HFT) environments. The architecture prioritizes **cache locality**, **zero-allocation on the critical path**, and **static polymorphism** to minimize instruction overhead.

## Key Features

### 1. Low-Latency Architecture
* **Custom Memory Pooling:** Implements an `ObjectPool` to pre-allocate memory for orders. This eliminates expensive `new`/`delete` calls during runtime and prevents heap fragmentation.
* **O(1) Order Book Operations:** Uses a `std::vector` indexed by price for price levels, allowing instant access to bid/ask queues without the $O(\log N)$ overhead of `std::map` or Red-Black trees.
* **Cache Friendliness:** The data structures are designed to keep hot data contiguous where possible, reducing cache misses.

### 2. Modern C++ Design
* **Static Polymorphism:** The `MatchingEngine` uses C++ templates for its Listener interface rather than virtual functions (`v-tables`), allowing the compiler to inline callbacks for maximum performance.
* **Concepts & Constraints:** Utilizes C++20 features to ensure type safety and cleaner code compilation.

### 3. Core Functionality
* **Price-Time Priority:** Orders are matched based on the standard FIFO algorithm (Price-Time).
* **Order Types:** Supports Limit Orders (Buy/Sell), Cancels, and Order Modifications (which maintain queue priority if the quantity decreases).
* **Infrastructure:** Includes a `LockFreeQueue` implementation (SPSC) ready for future integration into a multi-threaded consumer/producer model.

## Tech Stack
* **Language:** C++20
* **Build System:** CMake
* **Dependency Management:** Conan
* **Testing:** Google Test (GTest)
* **Benchmarking:** Google Benchmark

## Architecture Deep Dive

### The Matching Engine
The engine (`MatchingEngine.h`) is stateless and acts as a processor. It accepts an `OrderBook` and a `Listener`. It utilizes a policy-based design (`BuyPolicy` / `SellPolicy`) to handle side-specific logic without code duplication, resolved at compile-time.

### The Order Book
The book (`OrderBook.h`) manages the state of the market.
* **Levels:** Represents price levels as a doubly-linked list of orders. This allows for $O(1)$ insertion at the tail and $O(1)$ deletion from anywhere (essential for canceling orders).
* **Storage:** A `std::vector<Level>` is used as a lookup table. While this consumes more memory than a map for sparse distributions, it offers superior lookup speed for dense ticking products.

## Performance Benchmarks
Benchmarks are provided using Google Benchmark to measure the latency of critical operations. 

> **Note:** The current benchmarks are **really basic and have been AI-generated**. They serve as a preliminary baseline and need improvement to accurately represent production-grade scenarios.

To run benchmarks:

    ./build/Release/benchmarks/orderbook_bench

## Performance

The following benchmarks measure the **core engine latency** (hot path) on a single CPU core. They exclude network I/O and OS jitter, isolating the performance of the matching logic and data structures.

| Operation | Latency | Throughput | Description |
| :--- | :--- | :--- | :--- |
| **Add Order** | **~29 ns** | ~34M ops/sec | Insert a resting limit order into the book. |
| **Match Order** | **~40 ns** | ~25M ops/sec | Process an aggressive order that matches immediately (Trade). |
| **Cancel Order** | **~41 ns** | ~24M ops/sec | Locate and remove an existing order by ID. |

*Environment: Apple M2 Pro / C++20 / macOS*

## Build Instructions

### Prerequisites
* CMake (3.15+)
* Conan (2.0+)
* C++20 compliant compiler (GCC/Clang/MSVC)

### Building the Project
This project uses the Conan CMake layout, which separates Debug and Release builds.

**1. Install Dependencies:**

    conan install . --build=missing -s build_type=Debug
    conan install . --build=missing -s build_type=Release

**2. Build (Release):**
Recommended for benchmarks and performance testing.

    cmake --preset conan-release
    cmake --build --preset conan-release

**3. Build (Debug):**
Recommended for development and running unit tests.

    cmake --preset conan-debug
    cmake --build --preset conan-debug

### Running Tests
Unit tests cover full matching logic, partial fills, order modifications, and object pool exhaustion.

> **Note:** These tests are **really basic and AI-generated**. While they verify the core logic, they need improvement and expansion to cover deeper edge cases and regression scenarios.

    cd build/Debug
    ctest --output-on-failure
    
    # Or run the executable directly:
    ./tests/EngineTests



## Future Improvements
* **Test Suite Expansion:** Rewrite and expand the AI-generated tests to include comprehensive edge cases and fuzz testing.
* **TCP Gateway:** Implement a FIX (Financial Information eXchange) gateway to accept orders over the network.
* **Market Data Feed:** Separate the listener output to broadcast market data updates (L2/L3 data).