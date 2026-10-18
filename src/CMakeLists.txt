add_subdirectory(${SRC_ALGORITHMS})

set(MODULE_FILES
  ${MODULES}/integer_keys_set.cppm
  ${MODULES}/parser.cppm
)
set(SOURCES
  ${SRC}/main.cpp
)

set(TARGET target)
add_executable(${TARGET})
target_sources(${TARGET}
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${MODULES}
    FILES
      ${MODULE_FILES}
  PRIVATE
    ${SOURCES}
)
target_link_libraries(${TARGET} PRIVATE algorithms CLI11::CLI11)
