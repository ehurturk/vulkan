#include "timer.hpp"

namespace Core {
Timer::Timer() : m_StartTime(Clock::now()), m_PrevTick(Clock::now()) {}

void Timer::start() {
    if (!m_Running) {
        m_Running = true;
        m_StartTime = Clock::now();
    }
}

void Timer::lap() {
    m_Lapping = true;
    m_LapTime = Clock::now();
}

bool Timer::is_running() const {
    return m_Running;
}
}  // namespace Core