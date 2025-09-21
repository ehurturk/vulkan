include(FetchContent)
FetchContent_Declare(
        googletest
        DOWNLOAD_EXTRACT_TIMESTAMP = true
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(run_tests
        engine_tests.cpp
        memory/stackalloc_tests.cpp
        memory/destackalloc_tests.cpp
        memory/poolalloc_tests.cpp
)

target_link_libraries(run_tests
        PRIVATE
        GTest::gtest_main
        engine
)

include(GoogleTest)
gtest_add_tests(TARGET run_tests)
