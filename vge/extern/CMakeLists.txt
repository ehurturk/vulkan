set(VMA_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(VMA_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(VMA)

add_library(extern_headers INTERFACE)

target_include_directories(extern_headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(extern_headers INTERFACE
    GPUOpen::VulkanMemoryAllocator
)
