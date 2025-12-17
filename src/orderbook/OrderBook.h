#pragma once

#include "Level.h"
#include "src/domain/Order.h"
#include "src/infrastructure/ObjectPool.h"

#include <algorithm>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class OrderBook {
  public:
    OrderBook(size_t capacity, Price max_price)
        : resting_orders_pool(capacity), bids(max_price + 1), max_bid(0), asks(max_price + 1), min_ask(max_price + 1) {}

    bool hasBids() { return max_bid > 0; }
    Price bestBid() { return max_bid; }
    Level& bestBidLevel() { return bids[max_bid]; }
    Level& bidLevel(Price price) { return bids[price]; }

    bool hasAsks() { return min_ask < asks.size(); }
    Price bestAsk() { return min_ask; }
    Level& bestAskLevel() { return asks[min_ask]; }
    Level& askLevel(Price price) { return asks[price]; }

    void decrementBidCursor() {
        while (max_bid > 0 && bids[max_bid].empty()) {
            max_bid--;
        }
    }

    void incrementAskCursor() {
        while (min_ask < asks.size() && asks[min_ask].empty()) {
            min_ask++;
        }
    }

    Level::RestingOrder* find(OrderId order_id) {
        auto it = resting_orders.find(order_id);
        if (it != resting_orders.end()) [[likely]] {
            return it->second;
        }
        return nullptr;
    }

    void clean(Level::RestingOrder* resting_order) {
        resting_orders.erase(resting_order->order.id);
        resting_orders_pool.deallocate(resting_order);
    }

    Level::RestingOrder* insertBid(Order& order) {
        Level::RestingOrder* resting_order = resting_orders_pool.allocate();
        resting_order->order = order;
        Level& level = bidLevel(order.price);
        level.add(resting_order);
        resting_orders.emplace(order.id, resting_order);
        if (order.price > max_bid) {
            max_bid = order.price;
        }
        return resting_order;
    }

    Level::RestingOrder* insertAsk(Order& order) {
        Level::RestingOrder* resting_order = resting_orders_pool.allocate();
        resting_order->order = order;
        Level& level = askLevel(order.price);
        level.add(resting_order);
        resting_orders.emplace(order.id, resting_order);
        if (order.price < min_ask) {
            min_ask = order.price;
        }
        return resting_order;
    }

    void fillAskOrder(Level& ask_level, Level::RestingOrder* ask, Quantity trade_quantity) {
        ask->order.quantity -= trade_quantity;
        ask_level.reduceQuantity(trade_quantity);
        if (ask->order.quantity == 0) {
            ask_level.pop();
            clean(ask);
            incrementAskCursor();
        }
    }
    void fillBidOrder(Level& bid_level, Level::RestingOrder* bid, Quantity trade_quantity) {
        bid->order.quantity -= trade_quantity;
        bid_level.reduceQuantity(trade_quantity);
        if (bid->order.quantity == 0) {
            bid_level.pop();
            clean(bid);
            decrementBidCursor();
        }
    }

    void removeAsk(Level::RestingOrder* ask) {
        Level& ask_level = askLevel(ask->order.price);
        ask_level.erase(ask);
        clean(ask);
        incrementAskCursor();
    }

    void removeBid(Level::RestingOrder* bid) {
        Level& bid_level = bidLevel(bid->order.price);
        bid_level.erase(bid);
        clean(bid);
        decrementBidCursor();
    }

    ObjectPool<Level::RestingOrder> resting_orders_pool;
    std::unordered_map<OrderId, Level::RestingOrder*> resting_orders; // todo: very expensive

    std::vector<Level> bids;
    Price max_bid;

    std::vector<Level> asks;
    Price min_ask;
};