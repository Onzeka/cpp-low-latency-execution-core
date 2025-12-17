#pragma once

#include <cassert>

#include "domain/Order.h"
/**
 * @brief Double Linked list
 */
class Level {
  public:
    struct RestingOrder {
        Order order;
        RestingOrder* prev;
        RestingOrder* next;
    };

    Level() {
        dummy_head = new RestingOrder;
        dummy_tail = new RestingOrder;

        dummy_tail->prev = dummy_head;
        dummy_tail->next = nullptr;

        dummy_head->next = dummy_tail;
        dummy_head->prev = nullptr;

        total_quantity = 0;
    }

    ~Level() {
        delete dummy_head;
        delete dummy_tail;
    }

    void add(RestingOrder* resting_order) {
        total_quantity += resting_order->order.quantity;

        dummy_tail->prev->next = resting_order;
        resting_order->prev = dummy_tail->prev;
        resting_order->next = dummy_tail;
        dummy_tail->prev = resting_order;
    }

    bool empty() { return dummy_head->next == dummy_tail; }

    void erase(RestingOrder* resting_order) {
        total_quantity -= resting_order->order.quantity;
        resting_order->next->prev = resting_order->prev;
        resting_order->prev->next = resting_order->next;
    }

    void pop() {
        assert(!empty());
        RestingOrder* head = dummy_head->next;
        assert(head->order.quantity == 0);
        dummy_head->next = head->next;
        head->next->prev = dummy_head;
    }

    void reduceQuantity(Quantity delta) { total_quantity -= delta; }
    Quantity getTotalQuantity() { return total_quantity; }

    RestingOrder* top() { return dummy_head->next; }

  private:
    RestingOrder* dummy_head;
    RestingOrder* dummy_tail;
    Quantity total_quantity;
};