#include "renderer.hpp"
#include "vulkan/vulkan_renderer.hpp"
#include "renderer/backend/opengl/opengl_renderer.hpp"
#include "core/assert.hpp"

#include <memory>

namespace Renderer {

std::unique_ptr<IRendererBackend> MakeRenderer(RendererBackendType t) {
    switch (t) {
    case RendererBackendType::Vulkan:
        return std::make_unique<VulkanRenderer>();
    case RendererBackendType::OpenGL:
        return std::make_unique<OpenGLRenderer>();
    default:
        ASSERT(false);
    }
    return {};
}

Renderer::Renderer(const RendererConfig &cfg) : m_Config(cfg), m_Backend(MakeRenderer(cfg.backend)) {}

Renderer::~Renderer() { shutdown(); }

void Renderer::initialize() {
    if (m_Backend)
        m_Backend->initialize(m_Config);
}
void Renderer::shutdown() {
    if (m_Backend)
        m_Backend->shutdown();
}
RendererBackendType Renderer::backend_type() const noexcept { return m_Config.backend; }

}
