cmake_minimum_required (VERSION 3.11)

# Опция для переключения между быстрым и полным тестом
# Использование: cmake -DBENCHMARK_FULL=ON ..
option(BENCHMARK_FULL "Run full benchmarks with more repetitions" OFF)

if(BENCHMARK_FULL)
    set(BENCH_REPETITIONS 10)
    message(STATUS "Benchmark mode: FULL (10 repetitions)")
else()
    set(BENCH_REPETITIONS 1)
    message(STATUS "Benchmark mode: FAST (1 repetition)")
endif()

# Include Google Benchmark
include(../cmake/GoogleBenchmark.cmake)

add_executable (VEBSetBenchmark benchmark.cpp)
target_link_libraries(VEBSetBenchmark benchmark::benchmark benchmark::benchmark_main)

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    target_link_libraries(VEBSetBenchmark Shlwapi)
endif()

# Include Google Test
include(../cmake/GoogleTest.cmake)

add_executable (VEBSetTests tests.cpp)
target_link_libraries(VEBSetTests gtest_main)
add_test(NAME VEBSet COMMAND VEBSetTests)

# --- Visualization Section ---

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(PLOT_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/plot_bench.py")
set(BENCHMARK_RESULTS "${CMAKE_CURRENT_BINARY_DIR}/results.json")

add_custom_target(InstallPythonDeps
        COMMAND ${Python3_EXECUTABLE} -m pip install pandas seaborn matplotlib
        COMMENT "Installing Python dependencies..."
)

add_custom_target(VEBSetPlot
        # Шаг 1: Запуск бенчмарка
        COMMAND $<TARGET_FILE:VEBSetBenchmark>
        --benchmark_out=${BENCHMARK_RESULTS}
        --benchmark_out_format=json
        --benchmark_repetitions=${BENCH_REPETITIONS}
        --benchmark_display_aggregates_only=false

        # Шаг 2: Запуск Python скрипта
        COMMAND ${Python3_EXECUTABLE} ${PLOT_SCRIPT} ${BENCHMARK_RESULTS}

        DEPENDS VEBSetBenchmark
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running benchmarks (Repetitions: ${BENCH_REPETITIONS}) and generating plots..."
        USES_TERMINAL
)