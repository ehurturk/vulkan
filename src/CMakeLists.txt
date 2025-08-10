# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Create engine library
add_library(engine STATIC ${ENGINE_SOURCES})

set(CMAKE_BUILD_TYPE Debug)


target_sources(engine
    PRIVATE
        # Core files
        core/logger.cpp
        core/application.cpp
        core/memory/destack_allocator.cpp
        
        # Platform files
        platform/platform.cpp
        platform/platform_linux.cpp
        platform/platform_win32.cpp
        platform/platform_macos.cpp
        platform/platform_glfw.cpp
        platform/window.cpp
        
        # Add other source files as your project grows
        renderer/backend/vulkan/vulkan_renderer.cpp
        renderer/backend/opengl/opengl_renderer.cpp
        renderer/backend/renderer.cpp
        # resource/...
)

set(ENGINE_HEADERS
    # Core headers
    core/assert.hpp
    core/logger.hpp
    core/application.hpp
    core/memory/destack_allocator.hpp
    defines.hpp

    types/game_types.hpp
    
    # Platform headers
    platform/platform.hpp
    platform/window.hpp
    
    # Add other header files as your project grows
    renderer/backend/renderer.hpp
    renderer/backend/vulkan/vulkan_renderer.hpp
    renderer/backend/opengl/opengl_renderer.hpp
    
    # resource/...
)

target_sources(engine
    INTERFACE
        ${ENGINE_HEADERS}
)

# Set include directories
target_include_directories(engine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}  # Makes src/ a public include dir
)

# Link dependencies
target_link_libraries(engine 
    PUBLIC 
        Vulkan::Vulkan 
        glfw
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_HEADERS} ${ENGINE_SOURCES})

set (CMAKE_BUILD_TYPE Debug)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(engine PRIVATE BUILD_DEBUG=1)
else()
    target_compile_definitions(engine PRIVATE BUILD_RELEASE=1)
endif()

# Output some debug info
message(STATUS "Engine sources: ${ENGINE_SOURCES}")
message(STATUS "Engine headers: ${ENGINE_HEADERS}")
