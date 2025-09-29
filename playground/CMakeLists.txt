set(PLAYGROUND_SOURCES
    main.cpp
)

add_executable(VulkanApp)

target_sources(VulkanApp
    PRIVATE
        ${PLAYGROUND_SOURCES}
)

target_link_libraries(VulkanApp
    PRIVATE
        engine
)

setup_platform_definitions(VulkanApp)
setup_compiler_settings(VulkanApp)

target_compile_definitions(VulkanApp PRIVATE
    APP_NAME="VulkanPlayground"
    APP_VERSION="${PROJECT_VERSION}"
)

set_target_properties(VulkanApp PROPERTIES
    OUTPUT_NAME "VulkanPlayground"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# add_custom_command(TARGET VulkanApp POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:VulkanApp>/assets
# )

if(PLATFORM_WINDOWS)
    # add_custom_command(TARGET VulkanApp POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #     $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:VulkanApp>
    # )
endif()

message(STATUS "Playground application configured: VulkanApp")
message(STATUS "Playground sources: ${PLAYGROUND_SOURCES}")

