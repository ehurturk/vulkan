#pragma once

#include <vector>
#include <deque>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <span>

namespace Core {
class JobSystem {
   public:
    using JobEntryPoint = std::function<uintptr_t()>;
    using JobCounter = std::atomic<int>;

    enum class Priority { LOW, NORMAL, HIGH, CRITICAL };

    struct JobDeclaration {
        JobCounter* counter = nullptr;
        uintptr_t param;
        JobEntryPoint entry_point;
        Priority priority;
    };

    JobSystem();
    ~JobSystem();

    void kickJob(const JobDeclaration& decl);
    void kickJobs(std::span<JobDeclaration> jobs);

    void kickJobAndWait(const JobDeclaration& decl);
    void kickJobsAndWait(std::span<JobDeclaration> jobs);

    void waitForCounter(JobCounter* counter);

   private:
    std::vector<std::thread> m_Threads;
    std::deque<JobDeclaration> m_JobQueue;
    std::mutex m_QueueMutex;
    std::condition_variable m_Cond;
    bool m_ShouldTerminate{false};
};
}  // namespace Core