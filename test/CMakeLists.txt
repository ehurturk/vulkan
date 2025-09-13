include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(run_tests
    # Add your test source files here
    engine_tests.cpp
    memory/memory_tests.cpp
)

target_link_libraries(run_tests
    PRIVATE
        GTest::gtest_main  # Provides a main() function and links gtest
        engine             # Your engine library
)

include(GoogleTest)
gtest_add_tests(TARGET run_tests)
