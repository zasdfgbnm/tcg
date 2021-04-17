cmake_minimum_required(VERSION 3.16)

project(tcg)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/initialize.cpp
    src/utils.cpp
    src/command.cpp
    src/names.txt
    src/create.cpp
    src/freeze.cpp
    src/list.cpp
    src/set.cpp
    src/show.cpp
    src/self.cpp
    src/help.cpp
    src/tab-complete.cpp
    src/version.cpp
)

find_package(Boost 1.71.0 COMPONENTS filesystem REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Boost_FILESYSTEM_LIBRARY} fmt::fmt spdlog::spdlog)

target_compile_options(${PROJECT_NAME} PUBLIC -Werror -pedantic)

option(VERSION "Version of this project")
if(NOT VERSION)
  message("++ Version not specified, will try to infer version")

  # infer version
  execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/shells/xonsh/setup.py --version
                  OUTPUT_VARIABLE INFERRED_VERSION ERROR_QUIET)
  string(REGEX REPLACE "\n$" "" INFERRED_VERSION "${INFERRED_VERSION}")
  if("${INFERRED_VERSION}" STREQUAL "" OR "${INFERRED_VERSION}" STREQUAL "unknown")
    message("++ Unable to infer version")
  else()
    set(VERSION "${INFERRED_VERSION}")
    message("++ Version inferred as ${VERSION}")
  endif()

  # infer git commit
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse HEAD
                  OUTPUT_VARIABLE GIT_COMMIT ERROR_QUIET)
  string(REGEX REPLACE "\n$" "" GIT_COMMIT "${GIT_COMMIT}")
  if("${GIT_COMMIT}" STREQUAL "")
    message("++ Unknown git commit")
  else()
    message("++ Git commit is ${GIT_COMMIT}")
  endif()
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")

configure_file(src/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
