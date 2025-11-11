#pragma once

#include "renderer/backend/renderer.hpp"

namespace Renderer {

class OpenGLRenderer final : public RendererBackend {
   public:
    OpenGLRenderer();
    ~OpenGLRenderer() override;

    void initialize(const RendererConfig& cfg) override;
    void shutdown() override;
};
};  // namespace Renderer
