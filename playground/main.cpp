#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

#include <core/application.hpp>
#include <renderer/backend/renderer.hpp>
#include <platform/input/input.hpp>
#include <scene/camera.hpp>
#include <scene/camera_controller.hpp>

// TODO:
// 1) Rewrite the application extend logic, i think this is a bit confusing, idk.
// 2) I think all the renderer, window, and camera and input stuff should be in the base application
// class? 3) Rethink design choices:
//     - Does platform own application?
//     - Does platform own window?
//     - Who owns inupt manager? should it even be an instance, or can it be singleton (note
//     thread-safeness)?
//
// - Make delta time and frame time (and other AppStats) public and accesible via a singleton?
//      - no thread concern here, as they will be read only probably? -> return const
//
// - Add the ability to configure window size, title, mode (windowed vs fullscreen etc.) here.
//
// - Platform create input seems funny, change it to something reasonable.
//       -> maybe the platform owns the input system? seems more reasonable.
//
// - Decouple the renderer here, maybe do not expose the renderer at all!

class MyApp : public Core::Application {
public:
    MyApp() = default;

    bool initialize(Platform::Window* window) override {
        APP_LOG_INFO("[MyApp]: Initializing my app!");
        m_Window = window;

        m_Input = Platform::CreateInput(*m_Window);
        m_Input->setCursorMode(false, true);
        m_Input->setMouseMoveCallback([this](const Platform::MouseMoveEvent& event) {
            m_CameraController->process_mouse_move_event(event);
        });

        m_Camera = std::make_unique<Scene::Camera>(glm::vec3 { 0.0f, 2.0f, 3.0f });
        m_CameraController = std::make_unique<Scene::CameraController>(*m_Camera);
        m_CameraController->setMoveSpeed(5.0f);
        m_CameraController->setMouseSensitivity(0.1f);

        Renderer::RendererConfig rendererConfig;
        rendererConfig.backend = Renderer::RendererBackendType::Vulkan;
        rendererConfig.enableValidation = true;

        m_Renderer = std::make_unique<Renderer::Renderer>(m_Window, rendererConfig);
        m_Renderer->initialize();

        return true;
    }

    void update(float deltaTime) override {
        if (m_Input->isKeyPressed(Platform::KeyCode::Escape)) {
            requestClose();
        }

        m_Window->setTitle(
            "FPS: " + std::to_string(getFPS()) + ", MS: " + std::to_string(getFrameTime()));

        m_CameraController->update(*m_Input, deltaTime);
    }

    void render() override {
        Renderer::RenderContext context { .camera = m_Camera.get() };
        m_Renderer->draw_frame(context);
    }

    void cleanup() override { }

private:
    Platform::Window* m_Window;
    std::unique_ptr<Scene::Camera> m_Camera;
    std::unique_ptr<Scene::CameraController> m_CameraController;
    std::unique_ptr<Platform::Input> m_Input;
};

// TODO: change this entrypoint into a better "Cherno" style way.
// Main class defines the app, and the engine has an extern function that it calls.
ENTRYPOINT(platform) {
    MyApp app;
    platform.initialize();
    platform.run(&app);
    platform.terminate();

    return EXIT_SUCCESS;
}
