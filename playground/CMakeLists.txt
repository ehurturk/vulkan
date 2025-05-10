file(GLOB_RECURSE PLAYGROUND_SRC CONFIGURE_DEPENDS *.c)
message(STATUS "Found ${PLAYGROUND_SRC}")
add_executable(VulkanApp ${PLAYGROUND_SRC})
target_link_libraries(VulkanApp PRIVATE engine)

target_include_directories(VulkanApp PRIVATE ${CMAKE_SOURCE_DIR}/src)