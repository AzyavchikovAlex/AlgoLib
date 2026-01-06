cmake_minimum_required (VERSION 3.11)

# Include Google Benchmark
include(../cmake/GoogleBenchmark.cmake)


add_executable (VEBSetBenchmark benchmark.cpp)
target_link_libraries(VEBSetBenchmark benchmark::benchmark benchmark::benchmark_main)


# Link Shlwapi to the project
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    target_link_libraries(RadixBenchmark Shlwapi)
endif()

# Include Google Test
include(../cmake/GoogleTest.cmake)

add_executable (VEBSetTests tests.cpp)
target_link_libraries(VEBSetTests gtest_main)
add_test(NAME VEBSet COMMAND VEBSetTests)



#--- Visualization Section ---

# 1. Находим интерпретатор Python 3
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# 2. Определяем пути к скрипту и файлу результатов
# Предполагаем, что plot_bench.py лежит в той же папке, что и CMakeLists.txt
set(PLOT_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/plot_bench.py")
set(BENCHMARK_RESULTS "${CMAKE_CURRENT_BINARY_DIR}/results.json")

# 3. Создаем кастомную цель "VEBSetPlot"
# Запуск: make VEBSetPlot (или cmake --build . --target VEBSetPlot)
add_custom_target(VEBSetPlot
# Шаг 1: Запуск исполняемого файла бенчмарка
# $<TARGET_FILE:...> автоматически подставит правильный путь к exe файлу
COMMAND $<TARGET_FILE:VEBSetBenchmark>
--benchmark_out=${BENCHMARK_RESULTS}
--benchmark_out_format=json
--benchmark_repetitions=10
--benchmark_display_aggregates_only=false

# Шаг 2: Запуск Python скрипта для отрисовки
COMMAND ${Python3_EXECUTABLE} ${PLOT_SCRIPT} ${BENCHMARK_RESULTS}

# Настройки зависимости и рабочей директории
DEPENDS VEBSetBenchmark
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Running benchmarks and generating plots..."
USES_TERMINAL
)