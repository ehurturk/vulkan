#pragma once

#include <memory>

namespace Renderer {

enum class RendererBackend { Vulkan = 0, OpenGL = 1 };

struct RendererState {
    RendererBackend backend = RendererBackend::Vulkan;
};

class Renderer {
  public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    Renderer(Renderer &&) = default;
    Renderer &operator=(Renderer &&) = default;

    void initialize();
    void destroy();

    const RendererState &getState() const { return m_state; }

  private:
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
    RendererState m_state;
};

}