#pragma once

#include "defines.hpp"
#include "renderer/backend/renderer.hpp"
#include "platform/platform.hpp"
#include <memory>
#include <string>

namespace Core {

struct ApplicationConfig {
    I32 width;
    I32 height;
    std::string name;
    Renderer::RendererBackendType backend;
};

class Application {
  public:
    API static Application &getInstance();

    API B8 create(const ApplicationConfig &config);
    API B8 run();
    API void shutdown();

    [[nodiscard]] Renderer::Renderer *getRenderer() const;

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

  private:
    Application();
    ~Application() = default;

    struct AppSpec {
        B8 initialized = false;
        B8 running = false;
        B8 suspended = false;
    };

    std::unique_ptr<Renderer::Renderer> m_Renderer;
    ApplicationConfig m_AppCfg;
    Platform::Platform::State m_PlatformState;
    AppSpec m_Spec;
};

} // namespace Core