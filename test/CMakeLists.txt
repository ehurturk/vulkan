# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Automatically discover all test files
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*_tests.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*_tests.cpp"
)

# Create test executable
add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests
    PRIVATE
        engine
        GTest::gtest_main
        GTest::gtest
)

setup_platform_definitions(run_tests)
setup_compiler_settings(run_tests)

target_compile_definitions(run_tests PRIVATE
    TESTING=1
    GTEST_HAS_PTHREAD=1
)

set_target_properties(run_tests PROPERTIES
    OUTPUT_NAME "EngineTests"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Use gtest_discover_tests for automatic test discovery
include(GoogleTest)
gtest_discover_tests(run_tests
    DISCOVERY_TIMEOUT 30
    PROPERTIES
        TIMEOUT 60
)

message(STATUS "Test configuration complete")
message(STATUS "Discovered ${CMAKE_CURRENT_LIST_LENGTH} test sources automatically")
