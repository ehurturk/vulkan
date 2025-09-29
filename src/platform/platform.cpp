#include "platform.hpp"

#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"

namespace Platform {

Platform::Platform(const PlatformContext& context) : m_Context(context) {
    m_WindowProperties.title = "Vulkan Game Engine";
    m_WindowProperties.extent = {1280, 720};
    m_WindowProperties.resizable = true;
    m_WindowProperties.vsync = Window::Vsync::DEFAULT;
}

ExitCode Platform::initialize() {
    LOG_INFO("[Platform]:Initializing platform for {}", m_WindowProperties.title);

    createWindow(m_WindowProperties);

    if (!m_Window) {
        LOG_FATAL("[Platform]:Failed to create window");
        return ExitCode::FatalError;
    }

    LOG_INFO("[Platform]:Platform initialized successfully");
    return ExitCode::Success;
}

ExitCode Platform::run(std::unique_ptr<Core::Application> app) {
    if (!app) {
        LOG_ERROR("[Platform]:No application provided to run");
        return ExitCode::FatalError;
    }

    m_App = std::move(app);

    if (!m_App->initialize(m_Window.get())) {
        LOG_ERROR("[Platform]:Failed to initialize application: {}", m_App->getName());
        return ExitCode::FatalError;
    }

    LOG_INFO("[Platform]:Starting application: {}", m_App->getName());
    m_Running = true;

    ExitCode result = mainLoop();

    m_App->cleanup();
    LOG_INFO("[Platform]:Application finished with exit code: {}", static_cast<int>(result));

    return result;
}

ExitCode Platform::mainLoop() {
    m_Timer.start();

    while (m_Running && !m_App->shouldClose()) {
        processEvents();

        if (m_Window->shouldClose()) {
            break;
        }

        updateFrame();
    }

    return ExitCode::Success;
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
    LOG_INFO("[Platform]:Terminating platform");

    if (m_App) {
        m_App->cleanup();
        m_App.reset();
    }

    m_Window.reset();
    m_Running = false;
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