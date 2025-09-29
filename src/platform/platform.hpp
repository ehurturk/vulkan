#pragma once

#include <memory>

#include "core/PlatformContext.hpp"
#include "core/timer.hpp"
#include "window.hpp"

namespace Core {
class Application;
}

namespace Platform {

class InputEvent;

enum class ExitCode { Success = 0, Close, FatalError };

class Platform {
   public:
    Platform(const PlatformContext& context);
    virtual ~Platform() = default;

    ExitCode initialize();
    ExitCode run(Core::Application* app);
    void terminate();

    Window& getWindow();
    void setWindowProperties(const Window::Properties& properties);

    void setFocus(bool focused);
    void disableInputProcessing();

    void forceRender(bool always_render);
    void setFixedFPS(float fps);

    static const uint32_t MIN_WINDOW_WIDTH = 420;
    static const uint32_t MIN_WINDOW_HEIGHT = 320;

   protected:
    virtual void createWindow(const Window::Properties& properties) = 0;

    virtual void processEvents();
    virtual void handleInputEvent(const InputEvent& event);
    virtual void handleResize(uint32_t width, uint32_t height);
    std::unique_ptr<Window> m_Window;

   private:
    ExitCode mainLoop();
    void updateFrame();

    const PlatformContext& m_Context;

    Core::Application* m_App;

    Core::Timer m_Timer;

    Window::Properties m_WindowProperties;

    bool m_Running{false};
    bool m_Focused{true};
    bool m_ProcessInput{true};
    bool m_AlwaysRender{false};
    bool m_FixedFPS{false};
    float m_FixedFrameTime{0.016f};
};
}  // namespace Platform