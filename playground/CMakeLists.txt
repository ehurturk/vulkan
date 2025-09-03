# Create executable
add_executable(VulkanApp ${PLAYGROUND_SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(VulkanApp PRIVATE BUILD_DEBUG=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(VulkanApp PRIVATE BUILD_RELEASE=1)
endif()

target_sources(VulkanApp
    PRIVATE
        main.cpp)

# Link with engine library
target_link_libraries(VulkanApp PRIVATE engine)

