#pragma once

#include <type_traits>
#include <vector>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <span>
#include <atomic>

#include "core/logger.hpp"

namespace Core {
class JobPool {
   public:
    using JobEntryPoint = std::function<void(uintptr_t)>;
    using JobCounter = std::atomic<int>;

    enum class Priority { LOW = 0, NORMAL, HIGH, CRITICAL };

    struct JobDeclaration {
        JobCounter* counter = nullptr;
        uintptr_t param;
        JobEntryPoint entry_point;
        Priority priority = Priority::NORMAL;
    };

    struct JobPriorityComparator {
        bool operator()(const JobDeclaration& a, const JobDeclaration& b) const {
            return a.priority < b.priority;
        }
    };

    JobPool();
    ~JobPool();

    template <typename F>
    void kickJob(F&& job_func, JobCounter* ctr = nullptr, Priority p = Priority::NORMAL) {
        using DecayedF = std::decay_t<F>;
        DecayedF* lambdaptr = new DecayedF(std::forward<F>(job_func));

        JobDeclaration decl;
        decl.entry_point = &JobEntryPointWrapper<DecayedF>;
        decl.param = reinterpret_cast<uintptr_t>(lambdaptr);
        decl.counter = ctr;
        decl.priority = p;

        kickJob(decl);
    }
    void kickJob(JobDeclaration& decl);

    void kickJobs(std::span<JobDeclaration> jobs);

    void kickJobAndWait(const JobDeclaration& decl);
    void kickJobsAndWait(std::span<JobDeclaration> jobs);

    void waitForCounter(JobCounter* counter);

   private:
    void workerThreadLoop();

    template <typename F>
    static void JobEntryPointWrapper(uintptr_t param) {
        F* lambdaptr = reinterpret_cast<F*>(param);
        try {
            (*lambdaptr)();
        } catch (...) {
            CORE_LOG_FATAL("[JobPool]: Exception occured in job!");
        }

        delete lambdaptr;
    }

    bool runSingleJob();

    std::vector<std::thread> m_Threads;
    std::priority_queue<JobDeclaration, std::vector<JobDeclaration>, JobPriorityComparator>
        m_JobQueue;
    mutable std::mutex m_QueueMutex;
    std::condition_variable m_Cond;
    std::atomic<bool> m_ShouldTerminate{false};
};
}  // namespace Core
