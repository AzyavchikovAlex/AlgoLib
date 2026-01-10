cmake_minimum_required (VERSION 3.11)
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    target_link_libraries(DSUBenchmark Shlwapi)
endif()

# --- Google Test ---
include(../cmake/GoogleTest.cmake)

add_executable(DSUTests tests.cpp)
target_link_libraries(DSUTests gtest_main)
add_test(NAME DSUTests_Run COMMAND DSUTests)