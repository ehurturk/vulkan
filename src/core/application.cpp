#include "application.hpp"
#include "core/logger.hpp"
#include "platform/platform.hpp"
#include "renderer/backend/renderer.hpp"
#include <memory>

namespace Core {
Application::Application():
#ifdef BUILD_DEBUG
    m_Renderer(std::make_unique<Renderer::Renderer>((Renderer::RendererConfig){.backend = Renderer::RendererBackendType::Vulkan, .enableValidation = true})),
#elif BUILD_RELEASE
    m_Renderer(std::make_unique<Renderer::Renderer>((Renderer::RendererConfig){.backend = Renderer::RendererBackendType::Vulkan, .enableValidation = false})),
#endif
    m_AppCfg({}), m_PlatformState({})
{

}

Application &Application::getInstance() {
    static Application instance;
    return instance;
}

b8 Application::create(const ApplicationConfig &config) {
    if (m_Spec.initialized) {
        LOG_ERROR("Application is already initialized. Can't create more than one application.");
        return false;
    }

    m_AppCfg = config;

    if (!Platform::Platform::startup(m_PlatformState, config.name, config.width, config.height)) {
        LOG_FATAL("Failed to start platform");
        return false;
    }

    m_Renderer->initialize();
    m_Spec.initialized = true;

    // TODO: Set renderer backend type here instead of ctor

    return true;
}

b8 Application::run() {
    if (!m_Spec.initialized) {
        LOG_ERROR("Application not initialized");
        return false;
    }

    m_Spec.running = true;

    while (Platform::Platform::shouldRun(m_PlatformState)) {
        Platform::Platform::dispatchMessages(m_PlatformState);
        // TODO: update
        // TODO: render
        // TODO: swap front & back buffers
    }

    m_Spec.running = false;
    Platform::Platform::shutdown(m_PlatformState);

    return true;
}

Renderer::Renderer *Application::getRenderer() const {
    return m_Renderer.get();
}

void Application::shutdown() {
    if (m_Spec.initialized) {
        if (m_Spec.running) {
            m_Spec.running = false;
        }
        if (m_Renderer) {
            m_Renderer->shutdown();
            m_Renderer.reset();
        }
        Platform::Platform::shutdown(m_PlatformState);
    } else {
        LOG_FATAL("SHUTDOWN ERROR: App not initialized!");
    }
}

} // namespace Core

