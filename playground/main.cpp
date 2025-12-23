#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

#include <core/application.hpp>
#include <renderer/backend/renderer.hpp>
#include <platform/input/input.hpp>
#include <scene/camera.hpp>
#include "platform/input/input_events.hpp"

// TODO:
// 1) Rewrite the application extend logic, i think this is a bit confusing, idk.
// 2) I think all the renderer, window, and camera and input stuff should be in the base application
// class? 3) Rethink design choices:
//     - Does platform own application?
//     - Does platform own window?
//     - Who owns inupt manager? should it even be an instance, or can it be singleton (note
//     thread-safeness)?

// TODO: decouple the renderer here, maybe do not expose the renderer at all!
class MyApp : public Core::Application {
   public:
    MyApp() : m_Camera{std::make_unique<Scene::Camera>(glm::vec3{0.0f, 0.0f, 3.0f})} {
        LOG_DEBUG("STARTING MYAPP!!!!!");
    }

    bool initialize(Platform::Window* window) override {
        LOG_INFO("[MyApp]: Initializing my app!");
        m_Window = window;

        m_Input = Platform::CreateInput(*m_Window);
        m_Input->setCursorMode(false);
        m_Input->setKeyCallback([this](const Platform::KeyEvent& e) {
            if (e.key == Platform::KeyCode::Escape) {
                requestClose();
            }
        });

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
    std::unique_ptr<Scene::Camera> m_Camera;
    std::unique_ptr<Platform::Input> m_Input;
};

ENTRYPOINT(platform) {
    MyApp app;

    platform.initialize();
    platform.run(&app);
    platform.terminate();

    return EXIT_SUCCESS;
}
