#pragma once
#include <cstdint>

using Price = uint64_t;
using Quantity = uint32_t;
using OrderId = uint64_t;
using Timestamp = uint64_t;

enum class Side { Buy, Sell };

struct Order {
    OrderId id;
    Quantity quantity;
    Price price;
    Side side;
};