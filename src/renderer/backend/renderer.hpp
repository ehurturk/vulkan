#pragma once
#include <memory>

namespace Renderer {

enum class RendererBackendType { Vulkan = 0, OpenGL = 1 };

struct RendererConfig {
    RendererBackendType backend = RendererBackendType::Vulkan;
    bool enableValidation = true;
};

class IRendererBackend {
  public:
    virtual ~IRendererBackend() = default;
    virtual void initialize(const RendererConfig &) = 0;
    virtual void shutdown() = 0;
};

class Renderer {
  public:
    explicit Renderer(const RendererConfig &cfg);
    ~Renderer();

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    Renderer(Renderer &&) noexcept = default;
    Renderer &operator=(Renderer &&) noexcept = default;

    void initialize();
    void shutdown();
    RendererBackendType backend_type() const noexcept;

  private:
    RendererConfig m_Config;
    std::unique_ptr<IRendererBackend> m_Backend;
};

std::unique_ptr<IRendererBackend> MakeRenderer(RendererBackendType);

} // namespace Renderer
