#include <gtest/gtest.h>
#include <resource/resource_pool.hpp>
#include <resource/handle.hpp>
#include <core/logger.hpp>

using namespace Resource;

struct MockResource {
    int value;
    std::string name;

    MockResource(int v = 0, std::string n = "")
        : value(v)
        , name(std::move(n)) { }
};

class ResourcePoolTest : public ::testing::Test {
protected:
    void SetUp() override { Core::Logger::initialize(); }

    void TearDown() override { Core::Logger::shutdown(); }

    ResourcePool<MockResource> pool;
};

TEST_F(ResourcePoolTest, AllocateAndGet) {
    Handle h = pool.allocate(MockResource { 42, "test" });

    ASSERT_TRUE(h);
    EXPECT_NE(h.value, 0u);

    MockResource* res = pool.get(h);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->value, 42);
    EXPECT_EQ(res->name, "test");
}

TEST_F(ResourcePoolTest, EmplaceInPlace) {
    Handle h = pool.emplace(100, "emplaced");

    MockResource* res = pool.get(h);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->value, 100);
    EXPECT_EQ(res->name, "emplaced");
}

TEST_F(ResourcePoolTest, InsertLvalue) {
    MockResource resource { 50, "lvalue" };
    Handle h = pool.insert(resource);

    MockResource* res = pool.get(h);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->value, 50);
    EXPECT_EQ(res->name, "lvalue");
}

TEST_F(ResourcePoolTest, InsertRvalue) {
    Handle h = pool.insert(MockResource { 75, "rvalue" });

    MockResource* res = pool.get(h);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->value, 75);
    EXPECT_EQ(res->name, "rvalue");
}

TEST_F(ResourcePoolTest, FreeInvalidatesHandle) {
    Handle h = pool.insert(MockResource { 10, "first" });

    ASSERT_NE(pool.get(h), nullptr);

    pool.free(h);

    // After freeing, the handle should be invalid
    EXPECT_EQ(pool.get(h), nullptr);
}

TEST_F(ResourcePoolTest, GenerationCounterIncrementsOnFree) {
    Handle h1 = pool.allocate(MockResource { 1, "gen1" });
    U32 gen1 = h1.gen();
    U32 idx = h1.index();

    pool.free(h1);

    // Allocate again in the same slot
    Handle h2 = pool.allocate(MockResource { 2, "gen2" });

    // Should be same index but different generation
    EXPECT_EQ(h2.index(), idx);
    EXPECT_EQ(h2.gen(), gen1 + 1);

    // Old handle should not work
    EXPECT_EQ(pool.get(h1), nullptr);

    // New handle should work
    MockResource* res = pool.get(h2);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->value, 2);
}

TEST_F(ResourcePoolTest, MultipleAllocationsUseDifferentSlots) {
    Handle h1 = pool.allocate(MockResource { 1, "a" });
    Handle h2 = pool.allocate(MockResource { 2, "b" });
    Handle h3 = pool.allocate(MockResource { 3, "c" });

    EXPECT_NE(h1.index(), h2.index());
    EXPECT_NE(h2.index(), h3.index());
    EXPECT_NE(h1.index(), h3.index());

    EXPECT_EQ(pool.get(h1)->value, 1);
    EXPECT_EQ(pool.get(h2)->value, 2);
    EXPECT_EQ(pool.get(h3)->value, 3);
}

TEST_F(ResourcePoolTest, FreedSlotIsReused) {
    Handle h1 = pool.allocate(MockResource { 10, "first" });
    U32 idx = h1.index();

    pool.free(h1);

    Handle h2 = pool.allocate(MockResource { 20, "second" });

    EXPECT_EQ(h2.index(), idx);
    EXPECT_EQ(pool.get(h2)->value, 20);
}

TEST_F(ResourcePoolTest, FreeAndReallocateMultipleTimes) {
    for (int i = 0; i < 10; ++i) {
        Handle h = pool.allocate(MockResource { i, "temp" });
        EXPECT_NE(pool.get(h), nullptr);
        pool.free(h);
    }

    // After all that churn, we should be able to allocate successfully
    Handle h = pool.allocate(MockResource { 999, "final" });
    EXPECT_EQ(pool.get(h)->value, 999);
}

TEST_F(ResourcePoolTest, SizeTracking) {
    EXPECT_EQ(pool.size(), 0u);

    Handle h1 = pool.allocate(MockResource { 1, "a" });
    EXPECT_EQ(pool.size(), 1u);

    Handle h2 = pool.allocate(MockResource { 2, "b" });
    EXPECT_EQ(pool.size(), 2u);

    pool.free(h1);
    EXPECT_EQ(pool.size(), 1u);

    pool.free(h2);
    EXPECT_EQ(pool.size(), 0u);
}

TEST_F(ResourcePoolTest, NullHandleReturnsNull) {
    Handle null = Handle::null();
    EXPECT_FALSE(null);
    EXPECT_EQ(null.value, 0u);
    EXPECT_EQ(pool.get(null), nullptr);
}

TEST_F(ResourcePoolTest, InvalidIndexReturnsNull) {
    Handle invalid = Handle::make(9999, 1);
    EXPECT_EQ(pool.get(invalid), nullptr);
}

TEST_F(ResourcePoolTest, OutdatedGenerationReturnsNull) {
    Handle h1 = pool.allocate(MockResource { 10, "first" });
    Handle old_handle = h1;

    pool.free(h1);
    Handle h2 = pool.allocate(MockResource { 20, "second" });

    // old_handle has outdated generation
    EXPECT_EQ(pool.get(old_handle), nullptr);

    // new handle should work
    EXPECT_NE(pool.get(h2), nullptr);
}

TEST_F(ResourcePoolTest, StressTestManyAllocations) {
    constexpr int N = 1000;
    std::vector<Handle> handles;

    for (int i = 0; i < N; ++i) {
        Handle h = pool.allocate(MockResource { i, "item" });
        ASSERT_TRUE(h);
        handles.push_back(h);
    }

    EXPECT_EQ(pool.size(), static_cast<size_t>(N));

    // Verify all handles are valid
    for (int i = 0; i < N; ++i) {
        MockResource* res = pool.get(handles[i]);
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->value, i);
    }

    for (Handle h : handles) {
        pool.free(h);
    }

    EXPECT_EQ(pool.size(), 0u);
}

TEST_F(ResourcePoolTest, StressTestAllocFreePattern) {
    std::vector<Handle> handles;

    // Allocate 100
    for (int i = 0; i < 100; ++i) {
        handles.push_back(pool.allocate(MockResource { i, "test" }));
    }

    // Free every other one
    for (size_t i = 0; i < handles.size(); i += 2) {
        pool.free(handles[i]);
    }

    EXPECT_EQ(pool.size(), 50u);

    // Allocate 50 more (should reuse freed slots)
    for (int i = 0; i < 50; ++i) {
        handles.push_back(pool.allocate(MockResource { i + 1000, "reuse" }));
    }

    EXPECT_EQ(pool.size(), 100u);
}
