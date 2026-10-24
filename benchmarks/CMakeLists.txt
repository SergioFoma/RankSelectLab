set(MODULE_FILES
  ${MODULES}/integer_keys_set.cppm
  ${MODULES}/parser.cppm
  ${MODULES}/configuring_paths.cppm
)

set(BENCHMARK_ANALYSIS benchmark_analysis)
add_executable(${BENCHMARK_ANALYSIS})
target_sources(${BENCHMARK_ANALYSIS}
  PUBLIC FILE_SET CXX_MODULES
    BASE_DIRS ${MODULES}
    FILES
      ${MODULE_FILES}
  PRIVATE
      ${BENCHMARKS}/benchmarks.cpp
)
target_link_libraries(${BENCHMARK_ANALYSIS} PRIVATE
  algorithms
  benchmark::benchmark
)


