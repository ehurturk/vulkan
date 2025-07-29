#pragma once

#include "../defines.hpp"
#include <memory>
#include <string_view>

namespace Platform {

class Platform {
  public:
    struct State {
        std::unique_ptr<void, void (*)(void *)> internalState{nullptr, [](void *) {}};
    };

    API static b8 startup(State &state, std::string_view name, i32 width, i32 height);
    API static void shutdown(State &state);
    API static b8 dispatchMessages(State &state);
    API static b8 shouldRun(State &state);

    static void consoleWrite(std::string_view msg, std::string_view color);

    static void *allocate(u64 size, b8 aligned);
    static void free(void *block, b8 aligned);
    static void *zeroMemory(void *block, u64 size);
    static void *copyMemory(void *dest, const void *source, u64 size);
    static void *setMemory(void *dest, i32 value, u64 size);

    static f64 getAbsoluteTime();
    static void sleep(u64 ms);
};

}