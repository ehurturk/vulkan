cmake_minimum_required(VERSION 3.16)

project(VulkanProject
    VERSION 0.1
    LANGUAGES C
)

# Set the C++ standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add source and external directories
add_subdirectory(src)
add_subdirectory(playground)
# Optionally add externals in the future
# add_subdirectory(extern)

