#pragma once

#include "defines.hpp"
#include <string>
#include <string_view>
#include <format>
#include <array>
#include <filesystem>

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

namespace Core {

enum class LogLevel : U8 { Fatal = 0, Error, Warn, Info, Debug, Trace };

class Logger {
   public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    B8 initialize();
    void shutdown();

    template <typename... Args>
    void log(LogLevel level, std::format_string<Args...> format, Args&&... args) {
        logOutput(level, std::vformat(format.get(), std::make_format_args(args...)));
    }

   private:
    /* deleted ctor, dtor, copy ctor */
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void logOutput(LogLevel level, const std::string& message);

    static constexpr std::array<std::string_view, 6> s_levelColors = {
        Colors::FATAL, Colors::ERROR, Colors::WARN, Colors::INFO, Colors::DEBUG, Colors::TRACE};

    static constexpr std::array<std::string_view, 6> s_levelNames = {
        "[FATAL]:", "[ERROR]:", "[WARNING]:", "[INFO]:", "[DEBUG]:", "[TRACE]:"};
};

// Logging macros
#ifdef BUILD_DEBUG
#define LOG_FATAL(format, ...) \
    ::Core::Logger::getInstance().log(::Core::LogLevel::Fatal, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) \
    ::Core::Logger::getInstance().log(::Core::LogLevel::Error, format, ##__VA_ARGS__)

#if LOG_WARN_ENABLED == 1
#define LOG_WARN(format, ...) \
    ::Core::Logger::getInstance().log(::Core::LogLevel::Warn, format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...) ((void)0)
#endif

#if LOG_INFO_ENABLED == 1
#define LOG_INFO(format, ...) \
    ::Core::Logger::getInstance().log(::Core::LogLevel::Info, format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...) ((void)0)
#endif

#if LOG_DEBUG_ENABLED == 1
#define LOG_DEBUG(format, ...) \
    ::Core::Logger::getInstance().log(::Core::LogLevel::Debug, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...) ((void)0)
#endif

#if LOG_TRACE_ENABLED == 1
#define LOG_TRACE(format, ...) \
    ::Core::Logger::getInstance().log(::Core::LogLevel::Trace, format, ##__VA_ARGS__)
#else
#define LOG_TRACE(format, ...) ((void)0)
#endif

#elif BUILD_RELEASE
#define LOG_FATAL(format, ...) ((void)0)
#define LOG_ERROR(format, ...) ((void)0)
#define LOG_WARN(format, ...) ((void)0)
#define LOG_INFO(format, ...) ((void)0)
#define LOG_DEBUG(format, ...) ((void)0)
#define LOG_TRACE(format, ...) ((void)0)
#endif
}  // namespace Core
