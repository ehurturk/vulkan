#include <gtest/gtest.h>
#include <core/concurrency/job_system.hpp>
#include <core/logger.hpp>

using namespace Core;

class JobPoolTest : public ::testing::Test {
   protected:
    JobPool job{};
};

TEST_F(JobPoolTest, KicksJob) {
    auto myjob = []() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CORE_LOG_INFO("Thread done.");
    };

    auto myjob2 = []() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        CORE_LOG_INFO("Thread2 done.");
    };

    JobPool::JobCounter ctr{};

    job.kickJob(myjob, &ctr, JobPool::Priority::NORMAL);
    job.kickJob(myjob2, &ctr, JobPool::Priority::CRITICAL);

    job.waitForCounter(&ctr);

    EXPECT_TRUE(true);
}
