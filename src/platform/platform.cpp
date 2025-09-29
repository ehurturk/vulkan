#include "platform.hpp"

#include <algorithm>
#include <vector>

#include "core/assert.hpp"
#include "core/logger.hpp"
#include "platform/layer/layer.hpp"

namespace Platform {
const uint32_t Platform::MIN_WINDOW_WIDTH = 420;
const uint32_t Platform::MIN_WINDOW_HEIGHT = 320;

Platform::Platform(const PlatformContext& context) {}

ExitCode Platform::initialize(const std::vector<Layer*>& plugins_) {
    m_Plugins = plugins_;

    for (auto const& plugin : m_Plugins) {
        plugin->setPlatform(this);
    }

    // plugins might set m_Close, so account for close request from plugins
    if (m_Close) {
        return ExitCode::Close;
    }

    createWindow(m_WindowProperties);

    if (!m_Window) {
        LOG_FATAL("[Platform]:Window creation failed!");
        return ExitCode::FatalError;
    }

    return ExitCode::Success;
}

void Platform::registerHooks(Layer* layer) {
    const auto& layer_hooks = layer->getHooks();
    for (auto hook : layer_hooks) {
        auto layerIterator = m_Hooks.find(hook);

        if (layerIterator == m_Hooks.end()) {
            auto r = m_Hooks.emplace(hook, std::vector<Layer*>{});
            ASSERT(r.second);
            layerIterator = r.first;
        }

        if (std::ranges::none_of(layerIterator->second, [layer](Layer* p) { return p == layer; })) {
            layerIterator->second.emplace_back(layer);
        }
    }

    if (std::ranges::none_of(m_ActiveLayers, [layer](auto p) { return p == layer; })) {
        m_ActiveLayers.emplace_back(layer);
    }
}

ExitCode Platform::mainLoopFrame() {
    try {
        if (!startApp()) {
            throw std::runtime_error("Failed to load requested application");
        }

        // Compensate for load times of the app by rendering the first frame pre-emptively
        m_Timer.tick<Core::Timer::Seconds>();
        m_App->update(0.01667f);

        if (!m_App) {
            return ExitCode::NoSample;
        }

        update();

        if (m_App->shouldClose()) {
            std::string id = m_App->getName();
            onAppClose(id);
            m_App->finish();
        }

        m_Window->process_events();

        if (m_Window->should_close() || m_Close) {
            return ExitCode::Close;
        }
    } catch (std::exception& e) {
        LOG_ERROR("Error Message: {}", e.what());
        LOG_ERROR("Failed when running application {}", m_App->getName());

        onAppError(m_App->getName());
    }

    return ExitCode::Success;
}

ExitCode Platform::mainLoop() {
    ExitCode exit_code = ExitCode::Success;
    while (exit_code == ExitCode::Success) {
        exit_code = mainLoopFrame();
    }

    return exit_code;
}

void Platform::update() {
    auto delta_time = static_cast<float>(m_Timer.tick<Core::Timer::Seconds>());

    if (m_Focused || m_AlwaysRender) {
        onUpdate(delta_time);

        if (m_FixedSimFps) {
            delta_time = m_SimFrameTime;
        }

        m_App->updateUIOverlay(delta_time,
                               [=, this]() { onUpdateUIOverlay(*m_App->getRenderer()); });
        m_App->update(delta_time);

        if (auto* app = dynamic_cast<VulkanSampleCpp*>(m_App.get())) {
            if (app->has_render_context()) {
                on_post_draw(reinterpret_cast<vkb::RenderContext&>(app->get_render_context()));
            }
        } else if (auto* app = dynamic_cast<VulkanSampleC*>(m_App.get())) {
            if (app->has_render_context()) {
                on_post_draw(app->get_render_context());
            }
        }
    }
}

void Platform::terminate(ExitCode code) {
    if (m_App) {
        std::string id = m_App->getName();
        onAppClose(id);
        m_App->finish();
    }

    m_App.reset();
    m_Window.reset();

    onPlatformClose();
}

void Platform::close() {
    if (m_Window) {
        m_Window->close();
    }

    m_Close = true;
}

void Platform::forceSimulationFPS(float fps) {
    m_FixedSimFps = true;
    m_SimFrameTime = 1 / fps;
}

void Platform::forceRender(bool should_always_render) {
    m_AlwaysRender = should_always_render;
}

void Platform::disableInputProc() {
    m_ProcInputEvents = false;
}

void Platform::setFocus(bool _focused) {
    m_Focused = _focused;
}

void Platform::setWindowProperties(const Window::OptionalProperties& properties) {
    m_WindowProperties.title =
        properties.title.has_value() ? properties.title.value() : m_WindowProperties.title;
    m_WindowProperties.mode =
        properties.mode.has_value() ? properties.mode.value() : m_WindowProperties.mode;
    m_WindowProperties.resizable = properties.resizable.has_value() ? properties.resizable.value()
                                                                    : m_WindowProperties.resizable;
    m_WindowProperties.vsync =
        properties.vsync.has_value() ? properties.vsync.value() : m_WindowProperties.vsync;
    m_WindowProperties.extent.width = properties.extent.width.has_value()
                                          ? properties.extent.width.value()
                                          : m_WindowProperties.extent.width;
    m_WindowProperties.extent.height = properties.extent.height.has_value()
                                           ? properties.extent.height.value()
                                           : m_WindowProperties.extent.height;
}

Core::Application& Platform::getApp() {
    ASSERT_MSG(m_App, "Application is not valid");
    return *m_App;
}

Core::Application& Platform::getApp() const {
    ASSERT_MSG(m_App, "Application is not valid");
    return *m_App;
}

Window& Platform::getWindow() {
    return *m_Window;
}

bool Platform::startApp() {
    if (m_App) {
        m_App->finish();
    }

    m_App = requested_app_info->create();

    if (!m_App) {
        LOG_FATAL("[Platform]:Failed to create a valid vulkan app.");
        return false;
    }

    m_App->set_name(sample_info->name);

    if (!m_App->prepare({false, m_Window.get()})) {
        LOG_ERROR("[Platform]:Failed to prepare vulkan app.");
        return false;
    }

    onAppStart(requested_app_info->id);

    return true;
}

void Platform::inputEvent(const InputEvent& input_event) {
    if (m_ProcInputEvents && m_App) {
        m_App->inputEvent(input_event);
    }

    if (input_event.get_source() == EventSource::Keyboard) {
        const auto& key_event = static_cast<const KeyInputEvent&>(input_event);

        if (key_event.get_code() == KeyCode::Back || key_event.get_code() == KeyCode::Escape) {
            close();
        }
    }
}

void Platform::resize(uint32_t width, uint32_t height) {
    auto extent = Window::Extent{std::max<uint32_t>(width, MIN_WINDOW_WIDTH),
                                 std::max<uint32_t>(height, MIN_WINDOW_HEIGHT)};
    if ((m_Window) && (width > 0) && (height > 0)) {
        auto actual_extent = m_Window->resize(extent);

        if (m_App) {
            m_App->resize(actual_extent.width, actual_extent.height);
        }
    }
}

#define HOOK(enum, func)                  \
    static auto res = hooks.find(enum);   \
    if (res != hooks.end()) {             \
        for (auto plugin : res->second) { \
            plugin->func;                 \
        }                                 \
    }

void Platform::onUpdate(float delta_time) {
    HOOK(Hook::OnUpdate, onUpdate(delta_time));
}

void Platform::onAppStart(const std::string& app_id) {
    HOOK(Hook::OnAppStart, onAppStart(app_id));
}

void Platform::onAppClose(const std::string& app_id) {
    HOOK(Hook::OnAppClose, onAppClose(app_id));
}

void Platform::onAppError(const std::string& app_id) {
    HOOK(Hook::OnAppError, onAppError(app_id));
}
void Platform::onPlatformClose() {
    HOOK(Hook::OnPlatformClose, onPlatformClose());
}

void Platform::onUpdateUIOverlay() {
    HOOK(Hook::OnUpdateUi, onUpdateUIOverlay());
}

#undef HOOK

}  // namespace Platform