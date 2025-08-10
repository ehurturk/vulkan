# Create executable
add_executable(VulkanApp ${PLAYGROUND_SOURCES})

set (CMAKE_BUILD_TYPE Debug)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(VulkanApp PRIVATE BUILD_DEBUG=1)
else()
    target_compile_definitions(VulkanApp PRIVATE BUILD_RELEASE=1)
endif()

target_sources(VulkanApp
    PRIVATE
        main.cpp)

# Link with engine library
target_link_libraries(VulkanApp PRIVATE engine)

