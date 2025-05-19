# Crucial Todo
- src/CmakeLists.txt: make glfw and vulkan exposure PRIVATE (testbed should be independent of them)

# Long term todo
- FFT Ocean simulation using tesellation shaders
- Rayleigh + Mie atmospheric scattering
- Octree-based terrain LOD system
- Physics integration


# App Design
- Engine is exported as a library
- Playground / Apps use the engine as a library

Example functionality:
```c
#include <engine.h>

static application_t app;

model_t* sponza;
font_t* font;

void initialize();
void run();
void shutdown();

int main() {
    register_application(&app, "Test App", 600, 800);

    application_config_t settings;
    settings.backend = RENDERER_BACKEND_VULKAN;
    
    set_application_settings(settings);
}

void initialize() {
    sponza = load_model_from_gltf("./assets/sponza.gltf");   
    font = load_font_from_ttf("./assets/TimesNewRoman.ttf");
    
    scene_add_model(sponza);
    scene_add_text("Hello World", font);
}

void run() {
    /* runtime things here? */
}

void shutdown() {
    delete_model(sponza);
    delete_font(font);

    /* 
        * Or the engine can have a scene object queue
        * that frees objects in shutdown, and objects 
        * can be added to the queue through sceene_add_ functions
    */
}


```