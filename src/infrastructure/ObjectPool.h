#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>

template <typename T> class ObjectPool {
  private:
    std::vector<T> store;
    std::vector<size_t> free_indices;

  public:
    ObjectPool(size_t size) {
        store.resize(size);
        for (size_t i = 0; i < size; ++i) {
            free_indices.push_back(i);
        }
    }

    T* allocate() {
        if (free_indices.empty()) [[unlikely]] {
            throw std::runtime_error("Pool exhausted!");
        }
        size_t index = free_indices.back();
        free_indices.pop_back();
        return &store[index];
    }

    void deallocate(T* object) {
        size_t index = object - &store[0];
        free_indices.push_back(index);
    }
};