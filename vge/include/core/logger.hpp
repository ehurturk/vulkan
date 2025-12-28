#pragma once

#include <memory>
#include <string_view>
#include <format>

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

namespace spdlog {
class logger;
}

namespace Core {

class Logger {
   public:
    using LoggerRef = std::shared_ptr<spdlog::logger>;

    enum class LogLevel { Fatal, Error, Warn, Info, Debug, Trace };
    enum class LogSource { Core, Client };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    static bool initialize();
    static void shutdown();

    static LoggerRef& get_core_logger();
    static LoggerRef& get_client_logger();

    static void log(LogSource source, LogLevel level, std::string_view message);

   private:
    /* deleted ctor, dtor, copy ctor */
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static LoggerRef s_CoreLogger;
    static LoggerRef s_ClientLogger;
};

#ifdef BUILD_DEBUG
#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Core Macros
#define CORE_LOG_FATAL(...)                                                               \
    ::Core::Logger::log(::Core::Logger::LogSource::Core, ::Core::Logger::LogLevel::Fatal, \
                        std::format(__VA_ARGS__))
#define CORE_LOG_ERROR(...)                                                               \
    ::Core::Logger::log(::Core::Logger::LogSource::Core, ::Core::Logger::LogLevel::Error, \
                        std::format(__VA_ARGS__))

#if LOG_WARN_ENABLED
#define CORE_LOG_WARN(...)                                                               \
    ::Core::Logger::log(::Core::Logger::LogSource::Core, ::Core::Logger::LogLevel::Warn, \
                        std::format(__VA_ARGS__))
#else
#define CORE_LOG_WARN(...) ((void)0)
#endif

#if LOG_INFO_ENABLED
#define CORE_LOG_INFO(...)                                                               \
    ::Core::Logger::log(::Core::Logger::LogSource::Core, ::Core::Logger::LogLevel::Info, \
                        std::format(__VA_ARGS__))
#else
#define CORE_LOG_INFO(...) ((void)0)
#endif

#if LOG_DEBUG_ENABLED
#define CORE_LOG_DEBUG(...)                                                               \
    ::Core::Logger::log(::Core::Logger::LogSource::Core, ::Core::Logger::LogLevel::Debug, \
                        std::format(__VA_ARGS__))
#else
#define CORE_LOG_DEBUG(...) ((void)0)
#endif

#if LOG_TRACE_ENABLED
#define CORE_LOG_TRACE(...)                                                               \
    ::Core::Logger::log(::Core::Logger::LogSource::Core, ::Core::Logger::LogLevel::Trace, \
                        std::format(__VA_ARGS__))
#else
#define CORE_LOG_TRACE(...) ((void)0)
#endif

// Client Macros
#define APP_LOG_FATAL(...)                                                                  \
    ::Core::Logger::log(::Core::Logger::LogSource::Client, ::Core::Logger::LogLevel::Fatal, \
                        std::format(__VA_ARGS__))
#define APP_LOG_ERROR(...)                                                                  \
    ::Core::Logger::log(::Core::Logger::LogSource::Client, ::Core::Logger::LogLevel::Error, \
                        std::format(__VA_ARGS__))

#if LOG_WARN_ENABLED
#define APP_LOG_WARN(...)                                                                  \
    ::Core::Logger::log(::Core::Logger::LogSource::Client, ::Core::Logger::LogLevel::Warn, \
                        std::format(__VA_ARGS__))
#else
#define APP_LOG_WARN(...) ((void)0)
#endif

#if LOG_INFO_ENABLED
#define APP_LOG_INFO(...)                                                                  \
    ::Core::Logger::log(::Core::Logger::LogSource::Client, ::Core::Logger::LogLevel::Info, \
                        std::format(__VA_ARGS__))
#else
#define APP_LOG_INFO(...) ((void)0)
#endif

#if LOG_DEBUG_ENABLED
#define APP_LOG_DEBUG(...)                                                                  \
    ::Core::Logger::log(::Core::Logger::LogSource::Client, ::Core::Logger::LogLevel::Debug, \
                        std::format(__VA_ARGS__))
#else
#define APP_LOG_DEBUG(...) ((void)0)
#endif

#if LOG_TRACE_ENABLED
#define APP_LOG_TRACE(...)                                                                  \
    ::Core::Logger::log(::Core::Logger::LogSource::Client, ::Core::Logger::LogLevel::Trace, \
                        std::format(__VA_ARGS__))
#else
#define APP_LOG_TRACE(...) ((void)0)
#endif

#else
#define CORE_LOG_FATAL(...) ((void)0)
#define CORE_LOG_ERROR(...) ((void)0)
#define CORE_LOG_WARN(...) ((void)0)
#define CORE_LOG_INFO(...) ((void)0)
#define CORE_LOG_DEBUG(...) ((void)0)
#define CORE_LOG_TRACE(...) ((void)0)

#define APP_LOG_FATAL(...) ((void)0)
#define APP_LOG_ERROR(...) ((void)0)
#define APP_LOG_WARN(...) ((void)0)
#define APP_LOG_INFO(...) ((void)0)
#define APP_LOG_DEBUG(...) ((void)0)
#define APP_LOG_TRACE(...) ((void)0)
#endif
}  // namespace Core
