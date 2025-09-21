#pragma once

#include "../defines.hpp"
#include <string>

struct GLFWwindow;

namespace Platform {

class Window {
   public:
    struct Config {
        I32 width;
        I32 height;
        std::string name;
        B8 resizable;
        B8 fullscreen;
    };

    Window(const Config& config);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;

    B8 create();
    void pollEvents();
    B8 shouldClose() const;
    void shutdown();

    GLFWwindow* getHandle() const { return m_handle; }
    const Config& getConfig() const { return m_config; }

   private:
    GLFWwindow* m_handle = nullptr;
    Config m_config;
    static B8 s_glfwInitialized;
};

}  // namespace Platform