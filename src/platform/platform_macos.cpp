#include "platform.hpp"

//TODO: DO THIS
#ifdef REPLACETHISWITH_APPLE

namespace Platform {

struct InternalState {};

b8 Platform::startup(State &state, std::string_view name, i32 width, i32 height) {}

b8 Platform::shouldRun(State &state) {}

void Platform::shutdown(State &state) {}

b8 Platform::dispatchMessages(State &state) {}

void Platform::consoleWrite(std::string_view msg, std::string_view color) {}

f64 Platform::getAbsoluteTime() {}

void Platform::sleep(u64 ms) {}

void *Platform::allocate(u64 size, b8 aligned) { return std::malloc(size); }

void Platform::free(void *block, b8 aligned) { std::free(block); }

void *Platform::zeroMemory(void *block, u64 size) { return std::memset(block, 0, size); }

void *Platform::copyMemory(void *dest, const void *source, u64 size) {
    return std::memcpy(dest, source, size);
}

void *Platform::setMemory(void *dest, i32 value, u64 size) {
    return std::memset(dest, value, size);
}

}

#endif