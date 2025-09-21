#include "platform.hpp"

#ifdef PLATFORM_WNDOWS

namespace Platform {

struct InternalState {};

B8 Platform::startup(State& state, std::string_view name, I32 width, I32 height) {}

B8 Platform::shouldRun(State& state) {}

void Platform::shutdown(State& state) {}

B8 Platform::dispatchMessages(State& state) {}

void Platform::consoleWrite(std::string_view msg, std::string_view color) {}

F64 Platform::getAbsoluteTime() {}

void Platform::sleep(U64 ms) {}

void* Platform::allocate(U64 size, B8 aligned) {
    return std::malloc(size);
}

void Platform::free(void* block, B8 aligned) {
    std::free(block);
}

void* Platform::zeroMemory(void* block, U64 size) {
    return std::memset(block, 0, size);
}

void* Platform::copyMemory(void* dest, const void* source, U64 size) {
    return std::memcpy(dest, source, size);
}

void* Platform::setMemory(void* dest, I32 value, U64 size) {
    return std::memset(dest, value, size);
}

}  // namespace Platform

#endif