# Engine source files
set(ENGINE_SOURCES
    src/core/logger.cpp
    src/core/timer.cpp
    src/core/concurrency/job_system.cpp

    src/platform/platform.cpp
    src/platform/window/window.cpp
    src/platform/window/glfw_window.cpp
    src/platform/window/headless_window.cpp
    src/platform/input/glfw_input.cpp

    src/renderer/backend/renderer.cpp
    src/renderer/backend/shader_loader.cpp
    src/renderer/backend/vulkan/vulkan_renderer.cpp
    src/renderer/backend/vulkan/vulkan_utils.cpp
    src/renderer/backend/opengl/opengl_renderer.cpp

    src/resource/material_store.cpp
    src/resource/model_store.cpp
    src/resource/shader_store.cpp
    src/resource/texture_store.cpp

    src/scene/camera.cpp
    src/scene/camera_controller.cpp
)

# Platform-specific sources
if (PLATFORM_WINDOWS)
    list(APPEND ENGINE_SOURCES
        src/platform/windows/context.cpp
        src/platform/windows/entrypoint.cpp
        src/platform/windows/platform.cpp
    )
elseif (PLATFORM_LINUX OR PLATFORM_MACOS)
    list(APPEND ENGINE_SOURCES
        src/platform/unix/context.cpp
        src/platform/unix/entrypoint.cpp
        src/platform/unix/platform.cpp
    )
endif()

add_library(engine STATIC ${ENGINE_SOURCES})

target_compile_features(engine PUBLIC cxx_std_23)

target_include_directories(engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/extern
)

target_link_libraries(engine
    PUBLIC
        Vulkan::Vulkan
        glm::glm
    PRIVATE
        glfw
        assimp::assimp
        spdlog::spdlog
)

if (PLATFORM_LINUX)
    target_link_libraries(engine PRIVATE ${CMAKE_DL_LIBS} pthread)
elseif (PLATFORM_MACOS)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(engine PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
elseif (PLATFORM_WINDOWS)
    target_link_libraries(engine PRIVATE user32 gdi32 shell32)
endif()

setup_platform_definitions(engine)
setup_compiler_settings(engine)

target_compile_definitions(engine PRIVATE
    ENGINE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    ENGINE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    ENGINE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_VALIDATION_LAYERS)
    target_compile_definitions(engine PRIVATE ENABLE_VULKAN_VALIDATION=1)
endif()

set_target_properties(engine PROPERTIES
    OUTPUT_NAME "VulkanGameEngine"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    POSITION_INDEPENDENT_CODE ON
)

message(STATUS "VGE Engine configured with ${CMAKE_CXX_COMPILER_ID} compiler")
