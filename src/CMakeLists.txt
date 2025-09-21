find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_library(engine STATIC ${ENGINE_SOURCES}
        core/stl/MultipoolMemoryResource.hpp)

target_sources(engine
        PRIVATE
        # Core files
        core/logger.cpp
        core/application.cpp

        # Platform files
        platform/platform.cpp
        platform/platform_linux.cpp
        platform/platform_win32.cpp
        platform/platform_macos.cpp
        platform/platform_glfw.cpp
        platform/window.cpp

        renderer/backend/vulkan/vulkan_renderer.cpp
        renderer/backend/opengl/opengl_renderer.cpp
        renderer/backend/renderer.cpp
)

set(ENGINE_HEADERS
        # Core headers
        core/logger.hpp
        core/assert.hpp
        core/application.hpp
        defines.hpp

        core/memory/destack_allocator.hpp
        core/memory/stack_allocator.hpp
        core/memory/double_buffered_allocator.hpp
        core/memory/pool_allocator.hpp
        core/memory/scratch_allocator.hpp
        core/memory/memory.hpp
        core/memory/align_utils.hpp

        core/stl/FixedPoolResource.hpp
        core/stl/StackMemoryResource.hpp
        core/stl/DestackMemoryResource.hpp

        types/game_types.hpp

        # Platform headers
        platform/platform.hpp
        platform/window.hpp

        renderer/backend/renderer.hpp
        renderer/backend/vulkan/vulkan_renderer.hpp
        renderer/backend/opengl/opengl_renderer.hpp

        core/containers/static_vector.hpp

)

target_sources(engine
        INTERFACE
        ${ENGINE_HEADERS}
)


target_include_directories(engine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(engine
        PUBLIC
        Vulkan::Vulkan
        glfw
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_HEADERS} ${ENGINE_SOURCES})
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(engine PUBLIC BUILD_DEBUG=1)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(engine PUBLIC BUILD_RELEASE=1)
endif ()

# Output some debug info
message(STATUS "Engine sources: ${ENGINE_SOURCES}")
message(STATUS "Engine headers: ${ENGINE_HEADERS}")
