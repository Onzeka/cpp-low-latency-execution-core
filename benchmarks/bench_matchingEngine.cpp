#include <benchmark/benchmark.h>
#include <memory>
#include <vector>

#include "src/engines/MatchingEngine.h" 
#include "src/orderbook/OrderBook.h"

auto make_noop_listener() {
    return MatchingEngineListener{[](OrderId, OrderId, Price, Quantity) {}, [](const Order&) {}, [](OrderId) {}};
}

// ============================================================================
// BENCHMARK 1: Add Resting Orders
// ============================================================================
static void BM_AddRestingOrder(benchmark::State& state) {
    const size_t N = state.range(0);
    OrderBook book(N + 1000, 10000);
    MatchingEngine engine;
    auto listener = make_noop_listener();

    std::vector<Order> orders;
    orders.reserve(N);
    for (OrderId i = 1; i <= N; ++i) {
        // Alignment: {id, quantity, price, side}
        orders.push_back(Order{i, 100, 5000, Side::Sell});
    }

    size_t i = 0;
    for (auto _ : state) {
        engine.submitOrder(orders[i++], book, listener);
    }
}
BENCHMARK(BM_AddRestingOrder)->Arg(2000000)->Iterations(2000000);

// ============================================================================
// BENCHMARK 2: Match Orders
// ============================================================================
static void BM_MatchOrder(benchmark::State& state) {
    const size_t N = state.range(0);
    OrderBook book(N, 10000);
    MatchingEngine engine;
    auto listener = make_noop_listener();

    for (OrderId i = 1; i <= N; ++i) {
        engine.submitOrder(Order{i, 10, 5000, Side::Sell}, book, listener);
    }

    std::vector<Order> aggressive_orders;
    aggressive_orders.reserve(N);
    for (OrderId i = 0; i < N; ++i) {
        // Matches the resting Sell price of 5000
        aggressive_orders.push_back(Order{N + 1 + i, 10, 5000, Side::Buy});
    }

    size_t idx = 0;
    for (auto _ : state) {
        engine.submitOrder(aggressive_orders[idx++], book, listener);
    }
}
BENCHMARK(BM_MatchOrder)->Arg(2000000)->Iterations(2000000);

// ============================================================================
// BENCHMARK 3: Cancel Orders
// ============================================================================
static void BM_CancelOrder(benchmark::State& state) {
    const size_t N = state.range(0);
    OrderBook book(N, 10000);
    MatchingEngine engine;
    auto listener = make_noop_listener();

    std::vector<OrderId> ids_to_cancel;
    ids_to_cancel.reserve(N);

    for (OrderId i = 1; i <= N; ++i) {
        engine.submitOrder(Order{i, 100, 5000, Side::Sell}, book, listener);
        ids_to_cancel.push_back(i);
    }

    size_t idx = 0;
    for (auto _ : state) {
        engine.cancelOrder(ids_to_cancel[idx++], book, listener);
    }
}
BENCHMARK(BM_CancelOrder)->Arg(2000000)->Iterations(2000000);

BENCHMARK_MAIN();