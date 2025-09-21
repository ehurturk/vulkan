#pragma once

#include "../defines.hpp"
#include <memory>
#include <string_view>

namespace Platform {

class Platform {
   public:
    struct State {
        std::unique_ptr<void, void (*)(void*)> internalState{nullptr, [](void*) {}};
    };

    API static B8 startup(State& state, std::string_view name, I32 width, I32 height);
    API static void shutdown(State& state);
    API static B8 dispatchMessages(State& state);
    API static B8 shouldRun(State& state);

    static void consoleWrite(std::string_view msg, std::string_view color);

    static void* allocate(U64 size, B8 aligned);
    static void free(void* block, B8 aligned);
    static void* zeroMemory(void* block, U64 size);
    static void* copyMemory(void* dest, const void* source, U64 size);
    static void* setMemory(void* dest, I32 value, U64 size);

    static F64 getAbsoluteTime();
    static void sleep(U64 ms);
};

}  // namespace Platform