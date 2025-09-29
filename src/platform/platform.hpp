#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/PlatformContext.hpp"
#include "core/timer.hpp"
#include "core/application.hpp"
#include "platform/layer/layer.hpp"
#include "window.hpp"

namespace Platform {

class InputEvent;

enum class ExitCode {
    Success = 0, /* App executed as expected */
    Help,        /* App should show help */
    Close,       /* App has been requested to close at initialization */
    FatalError   /* App encountered an unexpected error */
};

class Platform {
   public:
    Platform(const PlatformContext& context);

    virtual ~Platform() = default;

    virtual ExitCode initialize(const std::vector<Layer*>& layers);

    ExitCode mainLoop();
    ExitCode mainLoopFrame();

    void update();

    virtual void terminate(ExitCode code);
    virtual void close();
    virtual void resize(uint32_t width, uint32_t height);
    virtual void inputEvent(const InputEvent& input_event);

    Window& getWindow();

    Core::Application& getApp() const;
    Core::Application& getApp();

    void setFocus(bool focused);
    bool startApp();
    void forceSimulationFPS(float fps);

    // force the application to always render even if it is not in focus
    void forceRender(bool should_always_render);

    void disableInputProc();

    void setWindowProperties(const Window::OptionalProperties& properties);

    static const uint32_t MIN_WINDOW_WIDTH;
    static const uint32_t MIN_WINDOW_HEIGHT;

   protected:
    std::vector<Layer*> m_ActiveLayers;
    std::unordered_map<Hook, std::vector<Layer*>> m_Hooks;

    std::unique_ptr<Window> m_Window{nullptr};

    std::unique_ptr<Core::Application> m_App{nullptr};

    virtual void createWindow(const Window::Properties& properties) = 0;

    void registerHooks(Layer* layer);

    void onUpdate(float delta_time);
    void onAppError(const std::string& app_id);
    void onAppStart(const std::string& app_id);
    void onAppClose(const std::string& app_id);
    void onPlatformClose();
    void onUpdateUIOverlay();

    Window::Properties m_WindowProperties;

    bool m_FixedSimFps{false};      // Delta time should be fixed with a fabricated value
    bool m_AlwaysRender{false};     // App should always render even if not in focus
    float m_SimFrameTime = 0.016f;  // A fabricated delta time
    bool m_ProcInputEvents{true};   // App should continue processing input events
    bool m_Focused{true};           // App is currently in focus at an operating system level
    bool m_Close{false};            // Close requested

    std::vector<Layer*> m_Plugins;

   private:
    Core::Timer m_Timer;
};

}  // namespace Platform