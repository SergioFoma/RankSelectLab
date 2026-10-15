set(MODULE_FILES
  ${MODULES}/algorithms/std_set_impl.cppm
  ${MODULES}/algorithms/sorted_vector_impl.cppm
  ${MODULES}/prog_error.cppm
)
set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/std_set_impl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sorted_vector_impl.cpp
)

add_library(algorithms STATIC)
target_sources(algorithms
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${MODULES}
    FILES
      ${MODULE_FILES}
  PRIVATE
    ${SOURCES}
)
target_link_libraries(algorithms PUBLIC TBB::tbb)
