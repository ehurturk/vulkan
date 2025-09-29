#pragma once

#include <string>
#include "defines.hpp"
#include "renderer/backend/renderer.hpp"

namespace Platform {
class Window;
};
namespace Core {
class InputEvent;

struct ApplicationOptions {
    bool benchmark_enabled{false};
    Platform::Window* window{nullptr};
};

class Application {
   public:
    Application();

    virtual ~Application() = default;

    virtual bool prepare(const ApplicationOptions& options);

    virtual void update(float dt);  // dt in seconds
    virtual void finish();

    virtual bool resize(const uint32_t width, const uint32_t height);

    virtual void inputEvent(const InputEvent& input_event);

    virtual Renderer::Renderer* getRenderer();

    const std::string& getName() const;
    void setName(const std::string& name);

    inline bool shouldClose() const { return m_ReqClose; }
    inline void requestClose() { m_ReqClose = true; }

   protected:
    float m_Fps{0.0f};
    float m_FrameTime{0.0f};  // ms

    U32 m_FrameCount{0};
    U32 m_LastFrameCount{0};

    bool m_LockSimSpeed{false};

    Platform::Window* m_Window{nullptr};

   private:
    std::string m_Name{};

    bool m_ReqClose{false};
};
}  // namespace Core