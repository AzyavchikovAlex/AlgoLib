# Required by FetchContent_MakeAvailable
cmake_minimum_required(VERSION 3.14)
project(AZLib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")

enable_testing()

add_subdirectory (RadixSort)
add_subdirectory (VanEmdeBoasSet)
add_subdirectory (DSU)

##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
##set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
