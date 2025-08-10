#pragma once

#include "defines.hpp"
#include "renderer/backend/renderer.hpp"
#include <memory>
#include <string>

namespace Core {

struct ApplicationConfig {
    i32 width;
    i32 height;
    std::string name;
    Renderer::RendererBackendType backend;
};

class Application {
  public:
    API static Application &getInstance();

    API b8 create(const ApplicationConfig &config);
    API b8 run();
    API void shutdown();

    Renderer::Renderer *getRenderer() const;

  private:
    Application() = default;
    ~Application() = default;
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
};

}