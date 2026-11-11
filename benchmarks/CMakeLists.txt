add_executable(benchmark_analysis)
target_compile_options(benchmark_analysis PRIVATE -Wall -O2)
target_sources(benchmark_analysis PRIVATE benchmarks.cpp)
target_link_libraries(benchmark_analysis PRIVATE
  prog_error
  integer_keys_set
  parser
  benchmark::benchmark
  CLI11::CLI11
)
