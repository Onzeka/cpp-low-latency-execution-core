#pragma once

#include "src/orderbook/OrderBook.h"

template <typename TradeCallback, typename AddCallback, typename CancelCallback, typename ModifyCallback>
struct MatchingEngineListener {

    TradeCallback trade_callback;
    AddCallback add_callback;
    CancelCallback cancel_callback;
    ModifyCallback modify_callback;

    void onTrade(OrderId incoming_id, OrderId resting_id, Price price, Quantity qty) {
        trade_callback(incoming_id, resting_id, price, qty);
    }

    void onOrderAdded(const Order& order) { add_callback(order); }

    void onOrderCanceled(OrderId id) { cancel_callback(id); }
    void onOrderModified(const Order& order) { modify_callback(order) }
};

struct MatchingEngine {

    template <typename MatchingEngineListener>
    static void submitOrder(Order order, OrderBook& book, MatchingEngineListener& listener) {
        if (order.side == Side::Buy) {
            BuyPolicy policy(book);
            match(order, policy, listener);
        } else {
            SellPolicy policy(book);
            match(order, policy, listener);
        }
    }
    template <typename MatchingEngineListener>
    static void cancelOrder(OrderId order_id, OrderBook& book, MatchingEngineListener& listener) {
        Level::RestingOrder* resting_order = book.find(order_id);
        if (resting_order->order.side == Side::Buy) {
            BuyPolicy policy(book);
            cancel(resting_order, policy, listener);
        } else {
            SellPolicy policy(book);
            cancel(resting_order, policy, listener);
        }
    }
    template <typename MatchingEngineListener>
    static void modifyOrder(OrderId order_id, Price price, Quantity quantity, OrderBook& book,
                            MatchingEngineListener& listener) {
        Level::RestingOrder* resting_order = book.find(order_id);
        if (resting_order->order.side == Side::Buy) {
            BuyPolicy policy(book);
            modify(resting_order, price, quantity, policy, listener);
        } else {
            SellPolicy policy(book);
            modify(resting_order, price, quantity, policy, listener);
        }
    }

  private:
    struct BuyPolicy {
        OrderBook& book;
        BuyPolicy(OrderBook& b) : book(b) {}

        bool hasMatchingOrders() { return book.hasAsks(); }

        bool canMatch(Order& buy_order) { return buy_order.price >= book.bestAsk(); }

        Level& matchLevel() { return book.bestAskLevel(); }

        Level& getRestingLevel(Price price) { return book.bidLevel(price); }

        void updateMatchableTOB() { book.incrementAskCursor(); }

        void updateRestingTOB() { book.decrementBidCursor(); }

        void insert(Order& buy_order) { book.insertBid(buy_order); }

        void fillRestingOrder(Level& bid_level, Level::RestingOrder* bid, Quantity trade_quantity) {
            book.fillBidOrder(bid_level, bid, trade_quantity);
        }

        void fillOppositeOrder(Level& ask_level, Level::RestingOrder* ask, Quantity trade_quantity) {
            book.fillAskOrder(ask_level, ask, trade_quantity);
        }

        void cancel(Level::RestingOrder* bid) { book.removeBid(bid); }
    };

    struct SellPolicy {
        OrderBook& book;
        SellPolicy(OrderBook& b) : book(b) {}

        bool hasMatchingOrders() { return book.hasBids(); }

        bool canMatch(Order& sell_order) { return sell_order.price <= book.bestBid(); }

        Level& matchLevel() { return book.bestBidLevel(); }

        Level& getRestingLevel(Price price) { return book.askLevel(price); }

        void insert(Order& sell_order) { book.insertAsk(sell_order); }

        void fillOppositeOrder(Level& bid_level, Level::RestingOrder* bid, Quantity trade_quantity) {
            book.fillBidOrder(bid_level, bid, trade_quantity);
        }

        void fillRestingOrder(Level& ask_level, Level::RestingOrder* ask, Quantity trade_quantity) {
            book.fillAskOrder(ask_level, ask, trade_quantity);
        }

        void cancel(Level::RestingOrder* ask) { book.removeAsk(ask); }
    };

    template <typename Policy, typename MatchingEngineListener>
    static void match(Order& order, Policy& book_policy, MatchingEngineListener& listener) {

        while (order.quantity > 0 && book_policy.canMatch(order) && book_policy.hasMatchingOrders()) {
            Level& match_level = book_policy.matchLevel();
            Level::RestingOrder* matching_order = match_level.top();

            Quantity trade_quantity = std::min(order.quantity, matching_order->order.quantity);
            order.quantity -= trade_quantity;
            book_policy.fillOppositeOrder(match_level, matching_order, trade_quantity);

            listener.onTrade(order.id, matching_order->order.id, matching_order->order.price, trade_quantity);
        }

        if (order.quantity > 0) {
            book_policy.insert(order);
            listener.onOrderAdded(order);
        }
    }

    template <typename Policy, typename MatchingEngineListener>
    static void cancel(Level::RestingOrder* resting_order, Policy& book_policy, MatchingEngineListener& listener) {
        OrderId id = resting_order->order.id;
        book_policy.cancel(resting_order);
        listener.onOrderCanceled(id);
    }

    template <typename Policy, typename MatchingEngineListener>
    static void modify(Level::RestingOrder* resting_order, Price price, Quantity quantity, Policy& book_policy,
                       MatchingEngineListener& listener) {

        if (price == resting_order->order.price && quantity < resting_order->order.quantity) {
            Level& order_level = book_policy.getRestingLevel(resting_order->order.price);
            Quantity delta = resting_order->order.quantity - quantity;
            book_policy.fillRestingOrder(order_level, resting_order, delta);
            listener.onOrderModified(resting_order->order);
        } else {
            Order modified_order = resting_order->order;
            modified_order.price = price;
            modified_order.quantity = quantity;
            book_policy.cancel(resting_order);
            match(modified_order, book_policy, listener);
        }
    }
};