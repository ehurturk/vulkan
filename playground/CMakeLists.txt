# Create executable
add_executable(VulkanApp ${PLAYGROUND_SOURCES})

target_sources(VulkanApp
    PRIVATE
        main.c)

# Link with engine library
target_link_libraries(VulkanApp PRIVATE engine)