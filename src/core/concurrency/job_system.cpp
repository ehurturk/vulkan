#include "job_system.hpp"
#include <atomic>
#include "defines.hpp"
#include "core/logger.hpp"

namespace Core {

JobPool::JobPool() {
    unsigned cores = std::thread::hardware_concurrency();
    CORE_LOG_INFO("[JobPool]: Initializing with {} threads", cores);

    for (uint i = 0; i < cores; i++) {
        m_Threads.emplace_back([this]() { workerThreadLoop(); });
    }
}

JobPool::~JobPool() {
    m_ShouldTerminate.store(true, std::memory_order_relaxed);
    m_Cond.notify_all();
    for (auto& t : m_Threads) {
        t.join();
    }
}

void JobPool::kickJob(JobDeclaration& decl) {
    if (decl.counter) {
        decl.counter->fetch_add(1, std::memory_order_acq_rel);
    }
    std::scoped_lock lock{m_QueueMutex};
    m_JobQueue.push(decl);
    m_Cond.notify_one();
}

void JobPool::kickJobs(std::span<JobDeclaration> jobs) {
    {
        std::scoped_lock lock{m_QueueMutex};
        for (auto& job : jobs) {
            m_JobQueue.push(job);
            job.counter->fetch_add(1, std::memory_order_acq_rel);
        }
    }
    m_Cond.notify_all();
}

void JobPool::kickJobAndWait(const JobDeclaration& decl) {
    // TODO
    (void)decl;
}

void JobPool::kickJobsAndWait(std::span<JobDeclaration> jobs) {
    // kick jobs with the same counter
    JobCounter ctr{static_cast<int>(jobs.size())};
    {
        std::scoped_lock lock{m_QueueMutex};
        for (auto& job : jobs) {
            job.counter = &ctr;
            m_JobQueue.push(job);
        }
    }

    m_Cond.notify_all();

    waitForCounter(&ctr);
}

void JobPool::waitForCounter(JobCounter* counter) {
    if (!counter) {
        return;
    }

    while (counter->load(std::memory_order_relaxed) > 0) {
        // run a job from the job queue
        runSingleJob();
        std::this_thread::yield();
    }
}

void JobPool::workerThreadLoop() {
    while (true) {
        JobPool::JobDeclaration decl;
        {
            std::unique_lock lock(m_QueueMutex);
            m_Cond.wait(lock, [this]() {
                return !m_JobQueue.empty() || (m_ShouldTerminate.load(std::memory_order_relaxed));
            });

            if (m_ShouldTerminate.load(std::memory_order_relaxed) && m_JobQueue.empty()) {
                return;
            }

            decl = std::move(m_JobQueue.top());
            m_JobQueue.pop();
        }

        decl.entry_point(decl.param);

        if (decl.counter) {
            decl.counter->fetch_sub(1, std::memory_order_acq_rel);
        }
    }
}

bool JobPool::runSingleJob() {
    JobPool::JobDeclaration decl;

    {
        std::scoped_lock lock{m_QueueMutex};
        if (m_JobQueue.empty()) {
            return false;
        }

        decl = std::move(m_JobQueue.top());
        m_JobQueue.pop();
    }

    decl.entry_point(decl.param);
    if (decl.counter) {
        decl.counter->fetch_sub(1, std::memory_order_acq_rel);
    }

    return true;
}

}  // namespace Core
