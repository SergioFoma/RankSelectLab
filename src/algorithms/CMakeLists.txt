set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/std_set_impl.cpp
)

add_library(algorithms STATIC ${SOURCES})
target_include_directories(algorithms PUBLIC ${INCLUDE})
