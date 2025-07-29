#include "logger.hpp"
#include "assert.hpp"
#include "platform/platform.hpp"
#include <format>

void assertion_report_failure(std::string_view expr, std::string_view msg, std::string_view file,
                              i32 line) {
    Core::Logger::getInstance().log(Core::LogLevel::Fatal,
                              "Assertion Failure: {}, message: {}, in file: {}, in line: {}", expr,
                              msg, file, line);
}
namespace Core {


b8 Logger::initialize() {
    // TODO: create a log file
    return true;
}

void Logger::shutdown() {
    // TODO: cleanup logging/write queued entries
}

void Logger::logOutput(LogLevel level, const std::string &message) {
    const size_t levelIndex = static_cast<size_t>(level);
    const bool isError = levelIndex < 2;

    std::string output = std::format("{}{}", s_levelNames[levelIndex], message);

    Platform::Platform::consoleWrite(output, std::string(s_levelColors[levelIndex]));
}

} // namespace cc