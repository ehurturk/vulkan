#pragma once

#include "job_system.hpp"
#include "core/logger.hpp"

namespace Core {

class CounterPool {
   public:
    CounterPool(U32 max_counters = 128) { m_Counters.reserve(max_counters); }

    ~CounterPool() {
        if (m_ActiveCounters > 0) {
            LOG_WARN("[CounterPool]:CounterPool destroyed with {} active counters!",
                     m_ActiveCounters.load());
        }
    }

    JobSystem::JobCounter* allocate(const std::string& debugName = "") {
        std::scoped_lock lock{m_Mutex};

        Counter* wrapper = nullptr;

        if (m_Available.empty()) {
            m_Counters.push_back(std::make_unique<Counter>());
            wrapper = m_Counters.back().get();
        } else {
            wrapper = m_Available.top();
            m_Available.pop();
        }

        wrapper->counter.store(0);
        wrapper->inUse = true;

#ifdef BUILD_DEBUG
        wrapper->debugName = debugName;
        wrapper->allocTime = std::chrono::steady_clock::now();
#endif

        m_ActiveCounters++;
        return &wrapper->counter;
    }

    void free(JobSystem::JobCounter* counter) {
        if (!counter)
            return;

        std::scoped_lock lock{m_Mutex};

        Counter* wrapper = nullptr;
        for (auto& w : m_Counters) {
            if (&w->counter == counter) {
                wrapper = w.get();
                break;
            }
        }

        if (!wrapper || !wrapper->inUse) {
            LOG_ERROR("[CounterPool]:Double-free or invalid counter!");
            return;
        }

        if (wrapper->counter.load() != 0) {
            LOG_WARN("[CounterPool]:Freeing counter with value {}", wrapper->counter.load());
        }

#ifdef BUILD_DEBUG
        auto duration = std::chrono::steady_clock::now() - wrapper->allocTime;
        LOG_DEBUG("[CounterPool]:Counter '{}' lived for {} ms", wrapper->debugName,
                  std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
#endif

        wrapper->inUse = false;
        m_Available.push(wrapper);
        m_ActiveCounters--;
    }

    size_t getActiveCount() const { return m_ActiveCounters; }

   private:
    struct Counter {
        JobSystem::JobCounter counter{0};
        bool inUse{false};

#ifdef BUILD_DEBUG
        std::string debugName;
        std::chrono::time_point<std::chrono::steady_clock> allocTime;
#endif
    };

    std::vector<std::unique_ptr<Counter>> m_Counters;
    std::stack<Counter*> m_Available;
    std::mutex m_Mutex;
    std::atomic<size_t> m_ActiveCounters{0};
};
}  // namespace Core
