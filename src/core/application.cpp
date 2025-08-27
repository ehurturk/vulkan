#include "application.hpp"
#include "core/logger.hpp"
#include "platform/platform.hpp"
#include "renderer/backend/renderer.hpp"
#include <memory>

namespace Core {


// Core application implementation
struct Application::Impl {
    Platform::Platform::State platformState;
    ApplicationConfig config;
    std::unique_ptr<Renderer::Renderer> renderer;
    b8 running = false;
    b8 suspended = false;
    b8 initialized = false;
};

Application &Application::getInstance() {
    static Application instance;
    return instance;
}

b8 Application::create(const ApplicationConfig &config) {
    if (m_pImpl) {
        LOG_ERROR("Application is already initialized. Can't create more than one application.");
        return false;
    }

    m_pImpl = std::make_unique<Impl>();
    m_pImpl->config = config;

    if (!Platform::Platform::startup(m_pImpl->platformState, config.name, config.width, config.height)) {
        LOG_FATAL("Failed to start platform");
        return false;
    }

#if BUILD_DEBUG
    m_pImpl->renderer = std::make_unique<Renderer::Renderer>(
        (Renderer::RendererConfig){.backend = config.backend, .enableValidation = true});
#elif BUILD_RELEASE
    m_pImpl->renderer = std::make_unique<Renderer::Renderer>(
        (Renderer::RendererConfig){.backend = config.backend, .enableValidation = false});
#endif

    m_pImpl->renderer->initialize();
    m_pImpl->initialized = true;

    return true;
}

b8 Application::run() {
    if (!m_pImpl || !m_pImpl->initialized) {
        LOG_ERROR("Application not initialized");
        return false;
    }

    m_pImpl->running = true;

    while (Platform::Platform::shouldRun(m_pImpl->platformState)) {
        Platform::Platform::dispatchMessages(m_pImpl->platformState);
        // TODO: update
        // TODO: render
        // TODO: swap front & back buffers
    }

    m_pImpl->running = false;
    Platform::Platform::shutdown(m_pImpl->platformState);

    return true;
}

Renderer::Renderer *Application::getRenderer() const {
    return m_pImpl ? m_pImpl->renderer.get() : nullptr;
}

void Application::shutdown() {
    if (m_pImpl) {
        if (m_pImpl->running) {
            m_pImpl->running = false;
        }
        if (m_pImpl->renderer) {
            m_pImpl->renderer->shutdown();
            m_pImpl->renderer.reset();
        }

        Platform::Platform::shutdown(m_pImpl->platformState);
        m_pImpl.reset();
    }
}

}
