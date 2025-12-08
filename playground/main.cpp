#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

#include <platform/unix/platform.hpp>
#include <core/application.hpp>
#include <renderer/backend/renderer.hpp>

class MyApp : public Core::Application {
   public:
    MyApp() { LOG_DEBUG("STARTING MYAPP!!!!!"); }

    bool initialize(Platform::Window* window) override {
        LOG_INFO("[MyApp]: Initializing my app!");
        m_Window = window;

        Renderer::RendererConfig rendererConfig;
        rendererConfig.backend = Renderer::RendererBackendType::Vulkan;
        rendererConfig.enableValidation = true;
        m_Renderer = std::make_unique<Renderer::Renderer>(m_Window, rendererConfig);

        m_Renderer->initialize();
        return true;
    }

    void update(float deltaTime) override {
        (void)deltaTime;
        m_Window->setTitle("FPS: " + std::to_string(getFPS()) +
                           ", MS: " + std::to_string(getFrameTime()));
    }

    void render() override { m_Renderer->draw_frame(); }

    void cleanup() override {}

   private:
    Platform::Window* m_Window;
};

ENTRYPOINT(platform) {
    MyApp app;

    platform.initialize();
    platform.run(&app);
    platform.terminate();

    return 1;
}
