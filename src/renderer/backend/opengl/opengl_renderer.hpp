#pragma once

#include "renderer/backend/renderer.hpp"

namespace Renderer {

class OpenGLRenderer final : public IRenderer {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer() override;

        void initialize(const RendererConfig &cfg) override;
        void shutdown() override;
};

};