cmake_minimum_required(VERSION 3.20)

project(VulkanGameEngine
        VERSION 0.1.0
        DESCRIPTION "Vulkan Game Engine"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (WIN32)
    set(PLATFORM_WINDOWS TRUE)
    set(PLATFORM_NAME "Windows")
elseif (UNIX AND NOT APPLE AND NOT ANDROID)
    set(PLATFORM_LINUX TRUE)
    set(PLATFORM_NAME "Linux")
elseif (APPLE)
    if (IOS)
        set(PLATFORM_IOS TRUE)
        set(PLATFORM_NAME "IOS")
    else ()
        set(PLATFORM_MACOS TRUE)
        set(PLATFORM_NAME "macOS")
    endif ()
elseif (ANDROID)
    set(PLATFORM_ANDROID TRUE)
    set(PLATFORM_NAME "Android")
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()

message(STATUS "Building for platform: ${PLATFORM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

function(setup_platform_definitions target)
    if (PLATFORM_WINDOWS)
        target_compile_definitions(${target} PRIVATE __PLATFORM_WINDOWS__=1)
        target_compile_definitions(${target} PRIVATE NOMINMAX)
        target_compile_definitions(${target} PRIVATE WIN32_LEAN_AND_MEAN)
    elseif (PLATFORM_LINUX)
        target_compile_definitions(${target} PRIVATE __PLATFORM_LINUX__=1)
    elseif (PLATFORM_MACOS)
        target_compile_definitions(${target} PRIVATE __PLATFORM_MACOS__=1)
    elseif (PLATFORM_IOS)
        target_compile_definitions(${target} PRIVATE __PLATFORM_IOS__=1)
    elseif (PLATFORM_ANDROID)
        target_compile_definitions(${target} PRIVATE __PLATFORM_ANDROID__=1)
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${target} PRIVATE BUILD_DEBUG=1)
        # target_compile_definitions(${target} PRIVATE DEBUG=1)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions(${target} PRIVATE BUILD_RELEASE=1)
        target_compile_definitions(${target} PRIVATE NDEBUG=1)
    endif ()
endfunction()

function(setup_compiler_settings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
                /W4                     # Warning level 4
                /WX                     # Treat warnings as errors
                /permissive-            # Strict C++ compliance
                /Zc:__cplusplus         # Enable correct __cplusplus macro
                /Zc:preprocessor        # Enable conforming preprocessor
        )
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${target} PRIVATE /MTd)
        else ()
            target_compile_options(${target} PRIVATE /MT)
        endif ()
    else ()
        target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                # -Wpedantic
                -Werror
                -Wno-unused-parameter
        )

        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${target} PRIVATE -g -O0)
        elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(${target} PRIVATE -O3 -DNDEBUG)
        endif ()
    endif ()
endfunction()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK.")
endif ()

if (NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW3 not found. Please install GLFW3.")
endif ()

message(STATUS "Found Vulkan: ${Vulkan_VERSION}")
message(STATUS "Found GLFW3: ${glfw3_VERSION}")

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_PLAYGROUND "Build playground application" ON)
option(ENABLE_VALIDATION_LAYERS "Enable Vulkan validation layers in debug builds" ON)

add_subdirectory(src)

if (BUILD_PLAYGROUND)
    add_subdirectory(playground)
endif ()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
