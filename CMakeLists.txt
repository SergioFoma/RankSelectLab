cmake_minimum_required(VERSION 3.28)

option(BUILD_TESTS "Building tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

project(rank_select_lab)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O2 -Wall")

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(BENCHMARKS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
set(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(SRC_ALGORITHMS ${SRC}/algorithms)

# === download utilities ====
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

FetchContent_Declare(
  cli11
  URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.7.2.zip
)

FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.5.zip
)

find_package(GTest)
find_package(CLI11)
find_package(benchmark)
find_package(TBB REQUIRED)

if (${BUILD_TESTS} AND NOT GTest_FOUND)
  FetchContent_MakeAvailable(googletest)
endif()
if (${BUILD_BENCHMARKS} AND NOT benchmark_FOUND)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googlebenchmark)
endif()
if (NOT CLI11_FOUND)
  FetchContent_MakeAvailable(cli11)
endif()

# === algorithms ===
add_subdirectory(${SRC})

# === google benchmarks ===
if (${BUILD_BENCHMARKS})
  add_subdirectory(${BENCHMARKS})
endif()

# === google tests ===
if (${BUILD_TESTS})
  add_subdirectory(${TESTS})
endif()


