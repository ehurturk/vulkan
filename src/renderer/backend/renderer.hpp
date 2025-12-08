#pragma once
#include <memory>

namespace Platform {
class Window;
}

namespace Renderer {

enum class RendererBackendType { Vulkan, OpenGL };

struct RendererConfig {
    RendererBackendType backend = RendererBackendType::Vulkan;
    bool enableValidation = true;
};

class RendererBackend {
   public:
    virtual ~RendererBackend() = default;
    virtual void initialize(const RendererConfig&) = 0;
    virtual void draw_frame() = 0;
    virtual void shutdown() = 0;
};

class Renderer {
   public:
    Renderer(Platform::Window* window, const RendererConfig& cfg);
    ~Renderer();

    // delete copy operations, default move operations
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) noexcept = default;
    Renderer& operator=(Renderer&&) noexcept = default;

    void initialize();
    void draw_frame();
    void shutdown();
    [[nodiscard]] RendererBackendType backend_type() const noexcept;

   private:
    RendererConfig m_Config;
    std::unique_ptr<RendererBackend> m_Backend;
};

std::unique_ptr<RendererBackend> CreateRendererBackend(Platform::Window&, RendererBackendType);

}  // namespace Renderer
