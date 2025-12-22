set(ENGINE_CORE_SOURCES
        core/logger.cpp
        core/timer.cpp

        core/concurrency/job_system.cpp
)

set(ENGINE_PLATFORM_SOURCES
        platform/platform.cpp
        platform/window.cpp
        platform/window/glfw_window.cpp
        platform/window/headless_window.cpp

)

if (PLATFORM_WINDOWS)
    list(APPEND ENGINE_PLATFORM_SOURCES platform/windows/context.cpp platform/windows/entrypoint.cpp platform/windows/platform.cpp)
elseif (PLATFORM_LINUX)
    list(APPEND ENGINE_PLATFORM_SOURCES platform/unix/context.cpp platform/unix/entrypoint.cpp platform/unix/platform.cpp)
elseif (PLATFORM_MACOS)
    list(APPEND ENGINE_PLATFORM_SOURCES platform/unix/context.cpp platform/unix/entrypoint.cpp platform/unix/platform.cpp)
endif ()

set(ENGINE_RENDERER_SOURCES
        renderer/backend/renderer.cpp
        renderer/backend/shader_loader.cpp
        renderer/backend/vulkan/vulkan_renderer.cpp
        renderer/backend/vulkan/vulkan_utils.cpp
        renderer/backend/opengl/opengl_renderer.cpp
)

set(ENGINE_SOURCES
        ${ENGINE_CORE_SOURCES}
        ${ENGINE_PLATFORM_SOURCES}
        ${ENGINE_RENDERER_SOURCES}
)

set(ENGINE_CORE_HEADERS
        core/application.hpp
        core/logger.hpp
        core/assert.hpp
        core/timer.hpp

        defines.hpp

        core/memory/memory.hpp
        core/memory/allocator_base.hpp
        core/memory/align_utils.hpp
        core/memory/stack_allocator.hpp
        core/memory/destack_allocator.hpp
        core/memory/pool_allocator.hpp
        core/memory/double_buffered_allocator.hpp

        core/stl/StackMemoryResource.hpp
        core/stl/DestackMemoryResource.hpp
        core/stl/FixedPoolResource.hpp
        core/stl/MultipoolMemoryResource.hpp
        core/stl/PoolAdapter.hpp

        core/concurrency/job_system.hpp
        core/concurrency/counter_pool.hpp

        core/containers/static_vector.hpp

        types/game_types.hpp
)

set(ENGINE_PLATFORM_HEADERS
        platform/platform.hpp
        platform/window.hpp
        platform/core/PlatformContext.hpp
        platform/core/PlatformEntryPoint.hpp
        platform/window/glfw_window.hpp
        platform/window/headless_window.hpp
)

if (PLATFORM_WINDOWS)
    list(APPEND ENGINE_PLATFORM_HEADERS platform/windows/context.hpp platform/windows/platform.hpp)
elseif (PLATFORM_LINUX)
    list(APPEND ENGINE_PLATFORM_HEADERS platform/unix/context.hpp platform/unix/platform.hpp)
elseif (PLATFORM_MACOS)
    list(APPEND ENGINE_PLATFORM_HEADERS platform/unix/context.hpp platform/unix/platform.hpp)
endif ()

set(ENGINE_RENDERER_HEADERS
        renderer/backend/renderer.hpp
        renderer/backend/shader_loader.hpp
        renderer/backend/vulkan/vulkan_renderer.hpp
        renderer/backend/vulkan/vulkan_types.hpp
        renderer/backend/vulkan/vulkan_descriptors.hpp
        renderer/backend/vulkan/vulkan_initializers.hpp
        renderer/backend/vulkan/vulkan_loader.hpp
        renderer/backend/vulkan/vulkan_pipelines.hpp
        renderer/backend/vulkan/vulkan_utils.hpp
        renderer/backend/opengl/opengl_renderer.hpp
)

set(ENGINE_EXTERN_HEADERS
        ../extern/stb_image.h
        ../extern/tiny_obj_loader.h
)

set(ENGINE_HEADERS
        ${ENGINE_CORE_HEADERS}
        ${ENGINE_PLATFORM_HEADERS}
        ${ENGINE_RENDERER_HEADERS}
)


include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/libs CACHE PATH "Missing description." FORCE)
FetchContent_Declare(assimp
GIT_REPOSITORY https://github.com/assimp/assimp.git
GIT_TAG master)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)


# engine target - build as static library

add_library(engine STATIC)

target_sources(engine
        PRIVATE
        ${ENGINE_SOURCES}
        ${ENGINE_EXTERN_HEADERS}
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES ${ENGINE_HEADERS}
)


target_include_directories(engine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        libs/assimp-src/include
)

target_link_libraries(engine
        PUBLIC
        Vulkan::Vulkan
        PRIVATE
        glfw
        assimp
)

if (PLATFORM_LINUX)
    target_link_libraries(engine PRIVATE
            ${CMAKE_DL_LIBS}
            pthread
    )
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
    target_link_libraries(engine PRIVATE
            user32
            gdi32
            shell32
    )
endif ()

setup_platform_definitions(engine)
setup_compiler_settings(engine)

target_compile_definitions(engine PRIVATE
        ENGINE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ENGINE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ENGINE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_VALIDATION_LAYERS)
    target_compile_definitions(engine PRIVATE ENABLE_VULKAN_VALIDATION=1)
endif ()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

set_target_properties(engine PROPERTIES
        OUTPUT_NAME "VulkanGameEngine"
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        POSITION_INDEPENDENT_CODE ON
)

message(STATUS "Engine target configured with ${CMAKE_CXX_COMPILER_ID} compiler")
message(STATUS "Engine sources: ${ENGINE_SOURCES}")
message(STATUS "Engine headers: ${ENGINE_HEADERS}")
message(STATUS "Platform definitions will be applied to engine target")
