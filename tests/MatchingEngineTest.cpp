#include <gtest/gtest.h>
#include <iostream>
#include <vector>

#include "src/engines/MatchingEngine.h"
#include "src/orderbook/OrderBook.h"

struct Event {
    enum Type { TRADE, ADDED, CANCELED } type;
    OrderId incoming_id;
    OrderId resting_id;
    Quantity qty;

    friend std::ostream& operator<<(std::ostream& os, const Event& e) {
        const char* type_str = (e.type == TRADE) ? "TRADE" : (e.type == ADDED ? "ADDED" : "CANCELED");
        return os << type_str << "(In:" << e.incoming_id << ", Rest:" << e.resting_id << ", Qty:" << e.qty << ")";
    }

    bool operator==(const Event& other) const {
        return type == other.type && incoming_id == other.incoming_id && resting_id == other.resting_id &&
               qty == other.qty;
    }
};

class MatchingEngineTest : public ::testing::Test {
  protected:
    OrderBook book{100, 10000};
    std::vector<Event> history;

    auto make_listener() {
        return MatchingEngineListener{
            [&](OrderId in, OrderId rest, Price p, Quantity q) { history.push_back({Event::TRADE, in, rest, q}); },
            [&](const Order& o) { history.push_back({Event::ADDED, o.id, 0, o.quantity}); },
            [&](OrderId id) { history.push_back({Event::CANCELED, id, 0, 0}); }};
    }

    void SetUp() override { history.clear(); }
};

TEST_F(MatchingEngineTest, FullMatch_RemovesLiquidity) {
    auto listener = make_listener();

    // Sell 50 @ 100
    // Struct: {id, quantity, price, side}
    MatchingEngine::submitOrder(Order{1, 50, 100, Side::Sell}, book, listener);
    history.clear();

    // Buy 50 @ 100
    MatchingEngine::submitOrder(Order{2, 50, 100, Side::Buy}, book, listener);

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0], (Event{Event::TRADE, 2, 1, 50}));
}

TEST_F(MatchingEngineTest, PartialMatch_RestsRemainder) {
    auto listener = make_listener();

    // Sell 50 @ 100
    MatchingEngine::submitOrder(Order{1, 50, 100, Side::Sell}, book, listener);
    history.clear();

    // Buy 60 @ 100
    MatchingEngine::submitOrder(Order{2, 60, 100, Side::Buy}, book, listener);

    ASSERT_EQ(history.size(), 2);
    EXPECT_EQ(history[0], (Event{Event::TRADE, 2, 1, 50}));
    EXPECT_EQ(history[1], (Event{Event::ADDED, 2, 0, 10}));
}

TEST_F(MatchingEngineTest, CancelOrder_ExplicitlyRemoves) {
    auto listener = make_listener();

    // Sell 100 @ 100
    MatchingEngine::submitOrder(Order{1, 100, 100, Side::Sell}, book, listener);
    history.clear();

    MatchingEngine::cancelOrder(1, book, listener);

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].type, Event::CANCELED);
}

TEST_F(MatchingEngineTest, ModifyInPlace_UpdatesWithoutLosingPriority) {
    auto listener = make_listener();

    // Sell 100 @ 100
    MatchingEngine::submitOrder(Order{1, 100, 100, Side::Sell}, book, listener);
    history.clear();

    // Modify ID 1: Price 100, New Qty 80
    MatchingEngine::modifyOrder(1, 100, 80, book, listener);

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].type, Event::ADDED);
    EXPECT_EQ(history[0].qty, 80);
}

TEST_F(MatchingEngineTest, ModifyAggressive_GhostTradeCheck) {
    auto listener = make_listener();

    // Sell 50 @ 100
    MatchingEngine::submitOrder(Order{1, 50, 100, Side::Sell}, book, listener);
    // Buy 50 @ 90
    MatchingEngine::submitOrder(Order{2, 50, 90, Side::Buy}, book, listener);
    history.clear();

    // Modify Buy #2 to Price 102 (Crosses Spread)
    MatchingEngine::modifyOrder(2, 102, 50, book, listener);

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].type, Event::TRADE);
    EXPECT_EQ(history[0].incoming_id, 2);
}