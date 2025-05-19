# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Create engine library
add_library(engine STATIC ${ENGINE_SOURCES})


target_sources(engine
    PRIVATE
        # Core files
        core/logger.c
        core/application.c
        
        # Platform files
        platform/platform.c
        platform/platform_linux.c
        platform/platform_win32.c
        platform/platform_macos.c
        platform/platform_glfw.c
        platform/window.c
        
        # Add other source files as your project grows
        # renderer/...
        # resource/...
)

set(ENGINE_HEADERS
    # Core headers
    core/assert.h
    core/cc.h
    core/logger.h
    core/application.h
    defines.h
    
    # Platform headers
    platform/platform.h
    platform/window.h
    
    # Add other header files as your project grows
    # renderer/...
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

# Output some debug info
message(STATUS "Engine sources: ${ENGINE_SOURCES}")
message(STATUS "Engine headers: ${ENGINE_HEADERS}")