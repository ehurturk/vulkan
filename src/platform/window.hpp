#pragma once

#include "../defines.hpp"
#include <string>

struct GLFWwindow;

namespace Platform {

class Window {
  public:
    struct Config {
        i32 width;
        i32 height;
        std::string name;
        b8 resizable;
        b8 fullscreen;
    };

    Window(const Config &config);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) = default;
    Window &operator=(Window &&) = default;

    b8 create();
    void pollEvents();
    b8 shouldClose() const;
    void shutdown();

    GLFWwindow *getHandle() const { return m_handle; }
    const Config &getConfig() const { return m_config; }

  private:
    GLFWwindow *m_handle = nullptr;
    Config m_config;
    static b8 s_glfwInitialized;
};

}