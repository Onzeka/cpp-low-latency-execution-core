#include "infrastructure/ObjectPool.h"
#include <gtest/gtest.h>

struct Dummy {
    int id;
    double price;
};

TEST(ObjectPoolTest, AllocationAndReuse) {
    ObjectPool<Dummy> pool(2);

    Dummy* dummy1 = pool.allocate();
    Dummy* dummy2 = pool.allocate();

    dummy1->id = 1;

    pool.deallocate(dummy1);
    Dummy* dummy3 = pool.allocate();

    ASSERT_NE(dummy1, dummy2);
    ASSERT_EQ(dummy3, dummy1);
    ASSERT_EQ(dummy3->id, 1);
}

TEST(ObjectPoolTest, PoolExhaustion) {
    ObjectPool<Dummy> pool(1);
    Dummy* dummy1 = pool.allocate();
    ASSERT_NE(dummy1, nullptr);
    ASSERT_THROW(pool.allocate(), std::runtime_error);
}