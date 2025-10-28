#include "opengl_renderer.hpp"
#include "renderer/backend/renderer.hpp"
#include "core/logger.hpp"
#include "core/assert.hpp"

namespace Renderer {

OpenGLRenderer::OpenGLRenderer() {
    LOG_FATAL("OpenGL Renderer is not implemented yet!");
    ASSERT(false);
}

OpenGLRenderer::~OpenGLRenderer() {}

void OpenGLRenderer::initialize(const RendererConfig& cfg) {
    (void)cfg;
}

void OpenGLRenderer::shutdown() {}

}  // namespace Renderer
