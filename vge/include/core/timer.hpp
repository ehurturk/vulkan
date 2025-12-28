#pragma once

#include <chrono>

namespace Core {
class Timer {
   public:
    using Seconds = std::ratio<1>;
    using Milliseconds = std::ratio<1, 1000>;
    using Microseconds = std::ratio<1, 1000000>;
    using Nanoseconds = std::ratio<1, 1000000000>;

    using Clock = std::chrono::high_resolution_clock;
    using DefaultResolution = Seconds;

    Timer();

    virtual ~Timer() = default;

    void start();

    void lap();

    template <typename T = DefaultResolution>
    double stop() {
        if (!m_Running) {
            return 0;
        }

        m_Running = false;
        m_Lapping = false;
        auto duration = std::chrono::duration<double, T>(Clock::now() - m_StartTime);
        m_StartTime = Clock::now();
        m_LapTime = Clock::now();

        return duration.count();
    }

    template <typename T = DefaultResolution>
    double elapsed() {
        if (!m_Running) {
            return 0;
        }

        Clock::time_point start = m_StartTime;

        if (m_Lapping) {
            start = m_LapTime;
        }

        return std::chrono::duration<double, T>(Clock::now() - start).count();
    }

    template <typename T = DefaultResolution>
    double tick() {
        auto now = Clock::now();
        auto duration = std::chrono::duration<double, T>(now - m_PrevTick);
        m_PrevTick = now;
        return duration.count();
    }

    bool is_running() const;

   private:
    bool m_Running{false};

    bool m_Lapping{false};

    Clock::time_point m_StartTime;

    Clock::time_point m_LapTime;

    Clock::time_point m_PrevTick;
};
}  // namespace Core