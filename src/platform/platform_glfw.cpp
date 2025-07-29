#include "platform.hpp"
#include "window.hpp"
#include "../core/logger.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <cstring>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

namespace Platform {

struct InternalState {
    std::unique_ptr<Window> window;
    b8 initialized = false;
};

b8 Platform::startup(State &state, std::string_view name, i32 width, i32 height) {
    std::unique_ptr<InternalState> internalState = std::make_unique<InternalState>();

    Window::Config winConfig{.width = width,
                             .height = height,
                             .name = std::string(name),
                             .resizable = false,
                             .fullscreen = false};

    internalState->window = std::make_unique<Window>(winConfig);

    if (!internalState->window->create()) {
        LOG_FATAL("Failed to create window!");
        return false;
    }

    internalState->initialized = true;
    state.internalState = std::unique_ptr<void, void (*)(void *)>(
        internalState.release(), [](void *ptr) { delete static_cast<InternalState *>(ptr); });

    return true;
}

b8 Platform::shouldRun(State &state) {
    InternalState *internalState = static_cast<InternalState *>(state.internalState.get());
    return internalState && internalState->window && !internalState->window->shouldClose();
}

void Platform::shutdown(State &state) {
    InternalState *internalState = static_cast<InternalState *>(state.internalState.get());
    if (internalState && internalState->initialized) {
        internalState->window.reset();
    }
    state.internalState.reset();
}

b8 Platform::dispatchMessages(State &state) {
    InternalState *internalState = static_cast<InternalState *>(state.internalState.get());
    internalState->window->pollEvents();
    return true;
}

void Platform::consoleWrite(std::string_view msg, std::string_view color) {
    std::cout << color << msg << Colors::Format::RESET << '\n';
}

f64 Platform::getAbsoluteTime() { return glfwGetTime(); }

void Platform::sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, nullptr);
#else
    if (ms >= 1000) {
        ::sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

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