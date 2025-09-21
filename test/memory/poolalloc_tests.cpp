#include <gtest/gtest.h>
#include <memory_resource>
#include <vector>
#include <list>
#include <unordered_map>
#include <thread>
#include <random>
#include <algorithm>

#include <core/memory/pool_allocator.hpp>
#include <core/stl/PoolMemoryResource.hpp>

using namespace Core;

struct Object {
    static inline std::atomic<int> constructCount{0};
    static inline std::atomic<int> destructCount{0};
    static inline std::atomic<int> copyCount{0};
    static inline std::atomic<int> moveCount{0};

    int value;
    std::string name;

    explicit Object(int v = 0, std::string n = "default") : value(v), name(std::move(n)) {
        ++constructCount;
    }

    Object(const Object& other) : value(other.value), name(other.name) { ++copyCount; }

    Object(Object&& other) noexcept : value(other.value), name(std::move(other.name)) {
        ++moveCount;
    }

    ~Object() { ++destructCount; }

    Object& operator=(const Object& other) {
        value = other.value;
        name = other.name;
        ++copyCount;
        return *this;
    }
    Object& operator=(Object&& other) noexcept {
        value = other.value;
        name = std::move(other.name);
        ++moveCount;
        return *this;
    }

    bool operator==(const Object& other) const {
        return value == other.value && name == other.name;
    }

    static void resetCounters() {
        constructCount = 0;
        destructCount = 0;
        copyCount = 0;
        moveCount = 0;
    }
};

class PoolAllocatorTest : public ::testing::Test {
   protected:
    void SetUp() override { Object::resetCounters(); }

    void TearDown() override {
        EXPECT_EQ(Object::constructCount + Object::copyCount + Object::moveCount,
                  Object::destructCount)
            << "Memory leak detected!";
    }
};

TEST_F(PoolAllocatorTest, WorksWithPmrVector) {
    PoolMemoryResource<Object> poolResource(1000);

    {
        std::pmr::vector<Object> vec(&poolResource);

        // Test push_back
        vec.push_back(Object(1, "one"));
        vec.push_back(Object(2, "two"));
        vec.push_back(Object(3, "three"));

        EXPECT_EQ(vec.size(), 3);
        EXPECT_EQ(vec[0].value, 1);
        EXPECT_EQ(vec[1].value, 2);
        EXPECT_EQ(vec[2].value, 3);

        // Test emplace_back
        vec.emplace_back(4, "four");
        EXPECT_EQ(vec.size(), 4);
        EXPECT_EQ(vec.back().value, 4);

        // Test iteration
        int sum = 0;
        for (const auto& obj : vec) {
            sum += obj.value;
        }
        EXPECT_EQ(sum, 10);  // 1+2+3+4

        // Test erase
        vec.erase(vec.begin() + 1);  // Remove "two"
        EXPECT_EQ(vec.size(), 3);
        EXPECT_EQ(vec[1].value, 3);  // "three" moved to index 1

        // Test clear
        vec.clear();
        EXPECT_EQ(vec.size(), 0);
        EXPECT_TRUE(vec.empty());
    }

    // After scope, check allocations match deallocations
    EXPECT_EQ(poolResource.getAllocCount(), poolResource.getDeallocCount());
}

// Test with list (better for pools as it allocates individual nodes)
TEST_F(PoolAllocatorTest, WorksWithPmrList) {
    PoolMemoryResource<Object> poolResource(100);

    {
        std::pmr::list<Object> list(&poolResource);

        // Lists allocate individual nodes - perfect for pools!
        for (int i = 0; i < 10; ++i) {
            list.emplace_back(i, "item_" + std::to_string(i));
        }

        EXPECT_EQ(list.size(), 10);

        // Test removal
        auto it = list.begin();
        std::advance(it, 5);
        list.erase(it);

        EXPECT_EQ(list.size(), 9);

        // Test find and modify
        auto found =
            std::find_if(list.begin(), list.end(), [](const Object& o) { return o.value == 7; });
        EXPECT_NE(found, list.end());
        found->name = "modified";

        // Test splice
        std::pmr::list<Object> list2(&poolResource);
        list2.emplace_back(100, "hundred");
        list.splice(list.end(), list2);

        EXPECT_EQ(list.size(), 10);
        EXPECT_EQ(list.back().value, 100);
    }

    EXPECT_EQ(poolResource.getAllocCount(), poolResource.getDeallocCount());
}

TEST_F(PoolAllocatorTest, WorksWithVector) {
    PoolAllocator<Object> alloc(100);
    std::vector<Object, PoolAllocator<Object>> myvec(alloc);
}

// Test with unordered_map
TEST_F(PoolAllocatorTest, WorksWithPmrUnorderedMap) {
    // For unordered_map, we need to handle pair<const Key, Value>
    using MapNode = std::pair<const int, Object>;
    PoolMemoryResource<MapNode> poolResource(100);

    {
        std::pmr::unordered_map<int, Object> map(&poolResource);

        // Insert elements
        for (int i = 0; i < 20; ++i) {
            map.emplace(i, Object(i * 10, "value_" + std::to_string(i)));
        }

        EXPECT_EQ(map.size(), 20);

        // Test lookup
        auto it = map.find(15);
        EXPECT_NE(it, map.end());
        EXPECT_EQ(it->second.value, 150);

        // Test erase
        map.erase(10);
        EXPECT_EQ(map.find(10), map.end());
        EXPECT_EQ(map.size(), 19);

        // Test iteration
        int keySum = 0;
        for (const auto& [key, value] : map) {
            keySum += key;
        }
        EXPECT_GT(keySum, 0);
    }
}

// Stress test with many operations
TEST_F(PoolAllocatorTest, StressTestWithVector) {
    PoolMemoryResource<Object> poolResource(10000);
    std::pmr::vector<Object> vec(&poolResource);

    // Random operations
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);
    std::uniform_int_distribution<> valueDis(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        int op = dis(gen);

        switch (op) {
            case 0:  // Add
                vec.emplace_back(valueDis(gen), "stress");
                break;
            case 1:  // Remove (if not empty)
                if (!vec.empty()) {
                    vec.pop_back();
                }
                break;
            case 2:  // Modify (if not empty)
                if (!vec.empty()) {
                    vec[vec.size() / 2].value = valueDis(gen);
                }
                break;
        }
    }

    // Verify integrity
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_GE(vec[i].value, 0);
        EXPECT_LE(vec[i].value, 1000);
    }
}

// Test capacity and reallocation behavior
TEST_F(PoolAllocatorTest, VectorCapacityAndReallocation) {
    PoolMemoryResource<Object> poolResource(10000);
    std::pmr::vector<Object> vec(&poolResource);

    // Reserve space
    vec.reserve(100);
    size_t initialCapacity = vec.capacity();
    EXPECT_GE(initialCapacity, 100);

    // Add elements without triggering reallocation
    for (int i = 0; i < 50; ++i) {
        vec.emplace_back(i, "item");
    }
    EXPECT_EQ(vec.capacity(), initialCapacity);

    // Force reallocation
    for (int i = 50; i < 200; ++i) {
        vec.emplace_back(i, "item");
    }
    EXPECT_GT(vec.capacity(), initialCapacity);
    EXPECT_EQ(vec.size(), 200);

    // Test shrink_to_fit
    vec.resize(10);
    vec.shrink_to_fit();
    EXPECT_LE(vec.capacity(), 20);  // Should be close to size
}

// Test exception safety
TEST_F(PoolAllocatorTest, ExceptionSafety) {
    // Small pool to trigger allocation failures
    PoolMemoryResource<Object> poolResource(10);
    std::pmr::vector<Object> vec(&poolResource);

    for (int i = 0; i < 100; ++i) {
        vec.push_back(Object(i, "test"));
    }

    // Vector should still be valid after exception
    EXPECT_FALSE(vec.empty());
    EXPECT_LE(vec.size(), 10);  // Should not exceed pool size

    // Should be able to continue using vector
    vec.clear();
    vec.push_back(Object(999, "after_exception"));
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].value, 999);
}

// Test move semantics
TEST_F(PoolAllocatorTest, MoveSemantics) {
    PoolMemoryResource<Object> poolResource(100);

    {
        std::pmr::vector<Object> vec1(&poolResource);
        vec1.emplace_back(1, "one");
        vec1.emplace_back(2, "two");

        // Move construct
        std::pmr::vector<Object> vec2(std::move(vec1));
        EXPECT_EQ(vec2.size(), 2);
        EXPECT_EQ(vec2[0].value, 1);

        // vec1 should be empty after move
        EXPECT_TRUE(vec1.empty());

        // Move assign
        std::pmr::vector<Object> vec3(&poolResource);
        vec3 = std::move(vec2);
        EXPECT_EQ(vec3.size(), 2);
        EXPECT_TRUE(vec2.empty());
    }
}

// Thread safety test for the pool itself
TEST_F(PoolAllocatorTest, ThreadSafetyWithList) {
    PoolMemoryResource<Object> poolResource(1000);

    const int numThreads = 4;
    const int opsPerThread = 100;
    std::vector<std::thread> threads;
    std::vector<std::pmr::list<Object>> lists(numThreads);

    // Each thread works with its own list but shares the pool
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t]() {
            lists[t] = std::pmr::list<Object>(&poolResource);

            for (int i = 0; i < opsPerThread; ++i) {
                lists[t].emplace_back(t * 1000 + i, "thread_" + std::to_string(t));

                // Occasionally remove elements
                if (i % 10 == 0 && !lists[t].empty()) {
                    lists[t].pop_front();
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Verify each list has expected elements
    for (int t = 0; t < numThreads; ++t) {
        EXPECT_GT(lists[t].size(), 0);
        for (const auto& obj : lists[t]) {
            EXPECT_GE(obj.value, t * 1000);
            EXPECT_LT(obj.value, (t + 1) * 1000);
        }
    }
}

// Test with sorting and algorithms
TEST_F(PoolAllocatorTest, AlgorithmsWithVector) {
    PoolMemoryResource<Object> poolResource(100);
    std::pmr::vector<Object> vec(&poolResource);

    // Add random elements
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);

    for (int i = 0; i < 20; ++i) {
        vec.emplace_back(dis(gen), "item");
    }

    // Sort
    std::sort(vec.begin(), vec.end(),
              [](const Object& a, const Object& b) { return a.value < b.value; });

    // Verify sorted
    for (size_t i = 1; i < vec.size(); ++i) {
        EXPECT_LE(vec[i - 1].value, vec[i].value);
    }

    // Test other algorithms
    auto it = std::find_if(vec.begin(), vec.end(), [](const Object& o) { return o.value > 50; });

    if (it != vec.end()) {
        EXPECT_GT(it->value, 50);
    }

    // Reverse
    std::reverse(vec.begin(), vec.end());

    // Verify reversed
    for (size_t i = 1; i < vec.size(); ++i) {
        EXPECT_GE(vec[i - 1].value, vec[i].value);
    }
}