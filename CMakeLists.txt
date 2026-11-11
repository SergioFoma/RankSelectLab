cmake_minimum_required(VERSION 3.28)

project(rank_select_lab)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === download utilities ====
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
  FIND_PACKAGE_ARGS NAMES GTest CONFIG
)
FetchContent_Declare(
  cli11
  URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.7.2.zip
  FIND_PACKAGE_ARGS NAMES CLI11 CONFIG
)
FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.5.zip
  FIND_PACKAGE_ARGS NAMES benchmark CONFIG
)
FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/releases/download/12.2.1/fmt-12.2.1.zip
  FIND_PACKAGE_ARGS NAMES fmt CONFIG
)
FetchContent_MakeAvailable(cli11)

find_package(TBB REQUIRED)

# === integer_keys_set ===
add_subdirectory(src)

# === google benchmarks ===
if (BENCHMARK_ENABLE_TESTING)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googlebenchmark)
  add_subdirectory(benchmarks)
endif()

include(CTest)
# === google tests ===
if (BUILD_TESTING)
  set(FMT_MODULE OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest fmt)
  enable_testing()
  add_subdirectory(tests)
endif()
