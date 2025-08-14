cmake_minimum_required (VERSION 3.11)

# Include Google Benchmark
include(../cmake/GoogleBenchmark.cmake)

add_executable (RadixBenchmark benchmark.cpp)
target_link_libraries(RadixBenchmark benchmark::benchmark benchmark::benchmark_main)

# Link Shlwapi to the project
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    target_link_libraries(RadixBenchmark Shlwapi)
endif()

# Include Google Test
include(../cmake/GoogleTest.cmake)

add_executable (RadixTests tests.cpp)
target_link_libraries(RadixTests gtest_main)
add_test(NAME RADIX COMMAND RadixTests)