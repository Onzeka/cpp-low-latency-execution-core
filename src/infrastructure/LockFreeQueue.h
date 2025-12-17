#pragma once
#include <atomic>
#include <vector>

template <typename T> class LockFreeQueue {

  public:
    LockFreeQueue(size_t size) {
        buffer.resize(size);
        head.store(0);
        tail.store(0);
    }

    bool push(const T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % buffer.size();

        if (next_head == tail.load(std::memory_order_acquire)) {
            return false;
        }

        buffer[current_head] = item;

        head.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);

        if (current_tail == head.load(std::memory_order_acquire)) {
            return false;
        }
        item = buffer[current_tail];

        size_t next_tail = (current_tail + 1) % buffer.size();

        tail.store(next_tail, std::memory_order_release);

        return true;
    }

  private:
    std::vector<T> buffer;

    // cache line separation (we make sure that these two variables are on differents cache lines)
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;
};