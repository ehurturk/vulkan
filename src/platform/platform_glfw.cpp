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
    B8 initialized = false;
};

B8 Platform::startup(State &state, std::string_view name, I32 width, I32 height) {
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

B8 Platform::shouldRun(State &state) {
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

B8 Platform::dispatchMessages(State &state) {
    InternalState *internalState = static_cast<InternalState *>(state.internalState.get());
    internalState->window->pollEvents();
    return true;
}

void Platform::consoleWrite(std::string_view msg, std::string_view color) {
    std::cout << color << msg << Colors::Format::RESET << '\n';
}

F64 Platform::getAbsoluteTime() { return glfwGetTime(); }

void Platform::sleep(U64 ms) {
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

void *Platform::allocate(U64 size, B8 aligned) { return std::malloc(size); }

void Platform::free(void *block, B8 aligned) { std::free(block); }

void *Platform::zeroMemory(void *block, U64 size) { return std::memset(block, 0, size); }

void *Platform::copyMemory(void *dest, const void *source, U64 size) {
    return std::memcpy(dest, source, size);
}

void *Platform::setMemory(void *dest, I32 value, U64 size) {
    return std::memset(dest, value, size);
}

}