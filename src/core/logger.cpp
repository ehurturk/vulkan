#include "logger.hpp"
#include "assert.hpp"
#include <iostream>
#include <format>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void assertion_report_failure(std::string_view expr,
                              std::string_view msg,
                              std::string_view file,
                              I32 line) {
    ::Core::Logger::get_core_logger()->critical(
        "Assertion Failure: {}, message: {}, in file: {}, in line: {}", expr, msg, file, line);
}
namespace Core {

Logger::LoggerRef Logger::s_CoreLogger;
Logger::LoggerRef Logger::s_ClientLogger;

bool Logger::initialize() {
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] [%n] %v");

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    // TODO: Make the application name retrievable from the system
    s_CoreLogger =
        std::make_shared<spdlog::logger>("VulkanRenderer", spdlog::sinks_init_list{console_sink});
    s_ClientLogger = std::make_shared<spdlog::logger>("APP", spdlog::sinks_init_list{console_sink});

    spdlog::register_logger(s_CoreLogger);
    spdlog::register_logger(s_ClientLogger);

    s_CoreLogger->set_level(spdlog::level::trace);
    s_ClientLogger->set_level(spdlog::level::trace);

    s_CoreLogger->info("Core logger initialized");
    s_ClientLogger->info("Client logger initialized");

    return true;
}

void Logger::shutdown() {
    spdlog::shutdown();
}

Logger::LoggerRef& Logger::get_core_logger() {
    return s_CoreLogger;
}

Logger::LoggerRef& Logger::get_client_logger() {
    return s_ClientLogger;
}

void Logger::log(LogSource source, LogLevel level, std::string_view message) {
    auto& logger = (source == LogSource::Core) ? s_CoreLogger : s_ClientLogger;

    switch (level) {
        case LogLevel::Fatal:
            logger->critical(message);
            break;
        case LogLevel::Error:
            logger->error(message);
            break;
        case LogLevel::Warn:
            logger->warn(message);
            break;
        case LogLevel::Info:
            logger->info(message);
            break;
        case LogLevel::Debug:
            logger->debug(message);
            break;
        case LogLevel::Trace:
            logger->trace(message);
            break;
    }
}

}  // namespace Core
