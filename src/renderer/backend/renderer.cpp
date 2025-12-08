#include "renderer.hpp"
#include "platform/platform.hpp"
#include "platform/window.hpp"
#include "vulkan/vulkan_renderer.hpp"
#include "opengl/opengl_renderer.hpp"
#include "core/assert.hpp"

#include <memory>

namespace Renderer {

std::unique_ptr<RendererBackend> CreateRendererBackend(Platform::Window* window,
                                                       RendererBackendType t) {
    switch (t) {
        case RendererBackendType::Vulkan:
            return std::make_unique<VulkanRenderer>(window);
        case RendererBackendType::OpenGL:
            return std::make_unique<OpenGLRenderer>();
        default:
            ASSERT(false);
    }
    return {};
}

Renderer::Renderer(Platform::Window* window, const RendererConfig& cfg)
    : m_Config(cfg), m_Backend(CreateRendererBackend(window, cfg.backend)) {}

Renderer::~Renderer() {
    shutdown();
}

void Renderer::initialize() {
    if (m_Backend)
        m_Backend->initialize(m_Config);
}
void Renderer::shutdown() {
    if (m_Backend)
        m_Backend->shutdown();
}
RendererBackendType Renderer::backend_type() const noexcept {
    return m_Config.backend;
}

void Renderer::draw_frame() {
    if (m_Backend)
        m_Backend->draw_frame();
}

}  // namespace Renderer
