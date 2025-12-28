#include "platform.hpp"

#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"
#include <iostream>
namespace Platform {

Platform::Platform(const PlatformContext& context)
    : m_Context{context}, m_App{nullptr}, m_Timer{}, m_WindowProperties{} {
    // TODO: parse this info from context arguments
    Core::Logger::initialize();

    m_WindowProperties.title = m_Context.arguments()[1];
    m_WindowProperties.extent = {1280, 720};
    m_WindowProperties.resizable = true;
    m_WindowProperties.vsync = Window::Vsync::DEFAULT;
}

bool Platform::initialize() {
    // CORE_LOG_INFO("[Platform]:Initializing platform for {}", m_WindowProperties.title);

    // Delagate the job to the correct platform to initialize the window
    createWindow(m_WindowProperties);

    if (!m_Window) {
        CORE_LOG_FATAL("[Platform]:Failed to create window");
        return false;
    }

    CORE_LOG_INFO("[Platform]:Platform initialized successfully");
    return true;
}

bool Platform::run(Core::Application* app) {
    if (!app) {
        CORE_LOG_ERROR("[Platform]:No application provided to run");
        return false;
    }

    m_App = app;

    if (!m_App->initialize(m_Window.get())) {
        CORE_LOG_ERROR("[Platform]:Failed to initialize application: {}", m_App->getName());
        return false;
    }

    CORE_LOG_INFO("[Platform]:Starting application: {}", m_App->getName());
    m_Running = true;

    bool result = mainLoop();

    m_App->cleanup();
    CORE_LOG_INFO("[Platform]:Application finished with exit code: {}", static_cast<int>(result));

    return result;
}

bool Platform::mainLoop() {
    m_Timer.start();

    while (m_Running && !m_App->shouldClose()) {
        processEvents();

        if (m_Window->shouldClose()) {
            break;
        }

        updateFrame();
    }

    return true;
}

void Platform::updateFrame() {
    float deltaTime = static_cast<float>(m_Timer.tick<Core::Timer::Seconds>());

    if (m_FixedFPS) {
        deltaTime = m_FixedFrameTime;
    }

    m_App->updatePerformanceStats(deltaTime);

    if (m_Focused || m_AlwaysRender) {
        m_App->update(deltaTime);
        m_App->render();
    }
}

void Platform::terminate() {
    CORE_LOG_INFO("[Platform]:Terminating platform");

    if (m_App) {
        m_App->cleanup();
    }

    m_Window.reset();
    m_Running = false;

    Core::Logger::shutdown();
}

Window& Platform::getWindow() {
    ASSERT_MSG(m_Window, "[Platform]:Window is not initialized");
    return *m_Window;
}

void Platform::setWindowProperties(const Window::Properties& properties) {
    m_WindowProperties = properties;

    if (m_Window) {
        m_Window->setTitle(properties.title);
        if (properties.extent.width > 0 && properties.extent.height > 0) {
            m_Window->resize(properties.extent);
        }
    }
}

void Platform::setFocus(bool focused) {
    m_Focused = focused;
}

void Platform::disableInputProcessing() {
    m_ProcessInput = false;
}

void Platform::forceRender(bool always_render) {
    m_AlwaysRender = always_render;
}

void Platform::setFixedFPS(float fps) {
    if (fps > 0.0f) {
        m_FixedFPS = true;
        m_FixedFrameTime = 1.0f / fps;
    } else {
        m_FixedFPS = false;
    }
}

void Platform::processEvents() {
    m_Window->processEvents();

    if (m_Window->shouldClose()) {
        m_Running = false;
    }
}

void Platform::handleInputEvent(const InputEvent& event) {
    if (m_ProcessInput && m_App) {
        m_App->onInputEvent(event);
    }
}

void Platform::handleResize(uint32_t width, uint32_t height) {
    width = std::max(width, MIN_WINDOW_WIDTH);
    height = std::max(height, MIN_WINDOW_HEIGHT);

    if (m_Window && width > 0 && height > 0) {
        Window::Extent extent{width, height};
        auto actual_extent = m_Window->resize(extent);

        if (m_App) {
            m_App->onResize(actual_extent.width, actual_extent.height);
        }
    }
}

}  // namespace Platform
