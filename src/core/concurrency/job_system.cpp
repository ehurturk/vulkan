#include "job_system.hpp"

Core::JobSystem::JobSystem() {}
Core::JobSystem::~JobSystem() {}
void Core::JobSystem::kickJob(const JobDeclaration& decl) {}
void Core::JobSystem::kickJobs(std::span<JobDeclaration> jobs) {}
void Core::JobSystem::kickJobAndWait(const JobDeclaration& decl) {}
void Core::JobSystem::kickJobsAndWait(std::span<JobDeclaration> jobs) {}
void Core::JobSystem::waitForCounter(JobCounter* counter) {}
