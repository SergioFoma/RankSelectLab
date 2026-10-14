cmake_minimum_required(VERSION 3.14)

option(BUILD_TESTS "Building tests" OFF)

project(rank_select_lab)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O2 -Wall")

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_ALGORITHMS ${SRC}/algorithms)

# === download utilities ====
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

find_package(GTest)
if (${BUILD_TESTS} AND NOT GTest_FOUND)
  FetchContent_MakeAvailable(googletest)
endif()

# === algorithms ===
add_subdirectory(${SRC})

# === google benchmarks ===

# === google tests ===
if (${BUILD_TESTS})
  add_subdirectory(${TESTS})
endif()


