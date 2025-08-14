set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable Google Test for benchmark")

set(BENCHMARK_ENABLE_TESTING NO)
include(FetchContent)
FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
)
FetchContent_MakeAvailable(googlebenchmark)
