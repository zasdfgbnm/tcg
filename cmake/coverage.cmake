# Allow obtaining code coverage data
option(CODE_COVERAGE "Enable coverage reporting" OFF)

if(CODE_COVERAGE)
  # gcda is the file generated at runtime
  # gcno is the file generated at compile time
  #
  # See:
  # https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html
  # https://gcc.gnu.org/onlinedocs/gcc-10.1.0/gcc/Instrumentation-Options.html
  option(GCDA_DIR "Argument for -fprofile-dir=" OFF)

  set(BUILD_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}.dir/src")
  set(CODECOV_CXX_FLAGS --coverage -fprofile-prefix-path=${BUILD_SRC_DIR})

  target_compile_options(${PROJECT_NAME} PUBLIC ${CODECOV_CXX_FLAGS})
  target_link_options(${PROJECT_NAME} PUBLIC ${CODECOV_CXX_FLAGS})
  if(NOT GCDA_DIR STREQUAL "OFF")
    target_compile_options(${PROJECT_NAME} PUBLIC -fprofile-dir=${GCDA_DIR})
    target_link_options(${PROJECT_NAME} PUBLIC -fprofile-dir=${GCDA_DIR})
  endif()
endif()
