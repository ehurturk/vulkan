# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB_RECURSE SRC_FILES *.c)
file(GLOB_RECURSE HEADER_FILES *.h)

# Create executable
# add_executable(VulkanApp ${SRC_FILES})
# target_include_directories(VulkanApp PRIVATE ${HEADER_FILES})
add_library(engine STATIC ${SRC_FILES} ${HEADER_FILES})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine PRIVATE Vulkan::Vulkan glfw)

# Include directories (if you plan to add headers later)
#target_include_directories(VulkanApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

