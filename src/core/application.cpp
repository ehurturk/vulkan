#include "application.hpp"
#include "core/assert.hpp"
#include "renderer/backend/renderer.hpp"

namespace Core {

Application::Application() : m_Name{"Sample Name"} {}

bool Application::prepare(const ApplicationOptions& options) {
    ASSERT_MSG(options.window != nullptr, "[Application]: Window must be present");

    m_LockSimSpeed = options.benchmark_enabled;
    m_Window = options.window;

    return true;
}

void Application::finish() {}

bool Application::resize(const uint32_t, const uint32_t) {
    return true;
}

void Application::inputEvent(const InputEvent& input_event) {}

Renderer::Renderer* Application::getRenderer() {
    return nullptr;
}

void Application::update(float delta_time) {
    m_Fps = 1.0f / delta_time;
    m_FrameTime = delta_time * 1000.0f;  // in ms
}

const std::string& Application::getName() const {
    return m_Name;
}

void Application::setName(const std::string& name_) {
    m_Name = name_;
}

}  // namespace Core