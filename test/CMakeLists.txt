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

set(TEST_SOURCES
    engine_tests.cpp
    
    memory/stackalloc_tests.cpp
    memory/destackalloc_tests.cpp
    memory/poolalloc_tests.cpp
    
    concurrency/threadpool_tests.cpp
)

add_executable(run_tests)

target_sources(run_tests
    PRIVATE
        ${TEST_SOURCES}
)

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

include(GoogleTest)
gtest_discover_tests(run_tests
    DISCOVERY_TIMEOUT 30
    PROPERTIES
        TIMEOUT 60
)

add_test(
    NAME engine_tests
    COMMAND run_tests
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_test(
    NAME memory_tests
    COMMAND run_tests --gtest_filter="*Memory*:*Allocator*"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_test(
    NAME concurrency_tests
    COMMAND run_tests --gtest_filter="*JobSystem*:*ThreadPool*"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

set_tests_properties(engine_tests PROPERTIES
    TIMEOUT 120
    PASS_REGULAR_EXPRESSION "PASSED"
)

message(STATUS "Test configuration complete")
message(STATUS "Test sources: ${TEST_SOURCES}")
message(STATUS "GoogleTest version: ${googletest_VERSION}")
