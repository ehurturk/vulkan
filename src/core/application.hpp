#pragma once

#include <string>
#include <cstdint>

namespace Platform {
class Window;
class InputEvent;
class Platform;
}  // namespace Platform

namespace Renderer {
class Renderer;
}

namespace Core {

class Application {
   public:
    Application() = default;
    virtual ~Application() = default;

    virtual bool initialize(Platform::Window* window) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void cleanup() = 0;

    virtual void onResize([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height) {}
    virtual void onInputEvent([[maybe_unused]] const Platform::InputEvent& event) {}

    bool shouldClose() const { return m_ShouldClose; }
    void requestClose() { m_ShouldClose = true; }

    const std::string& getName() const { return m_Name; }
    void setName(const std::string& name) { m_Name = name; }

    float getFPS() const { return m_FPS; }
    float getFrameTime() const { return m_FrameTime; }

   protected:
    Platform::Window* m_Window{nullptr};

   private:
    std::string m_Name{"Game Application"};
    bool m_ShouldClose{false};

    float m_FPS{0.0f};
    float m_FrameTime{0.0f};

    friend class Platform::Platform;
    void updatePerformanceStats(float deltaTime) {
        // delta time in ms
        m_FPS = 1.0f / deltaTime;
        m_FrameTime = deltaTime * 1000.0f;  // ms
    }
};
}  // namespace Core
