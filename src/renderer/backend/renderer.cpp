#include "renderer.hpp"
#include "./vulkan/vulkan_renderer.hpp"
#include "../../core/assert.hpp"

namespace Renderer {

struct Renderer::Impl {
    std::unique_ptr<VulkanRenderer> vulkanRenderer;
};

Renderer::Renderer() : m_pImpl(std::make_unique<Impl>()) {}

Renderer::~Renderer() { destroy(); }

void Renderer::initialize() {
    switch (m_state.backend) {
    case RendererBackend::Vulkan:
        m_pImpl->vulkanRenderer = std::make_unique<VulkanRenderer>();
        m_pImpl->vulkanRenderer->initialize(this);
        break;
    case RendererBackend::OpenGL:
        ASSERT_MSG(false, "OpenGL backend not implemented yet");
        break;
    }
}

void Renderer::destroy() {
    if (m_pImpl) {
        m_pImpl->vulkanRenderer.reset();
    }
}

}