cmake_minimum_required(VERSION 3.16)

project(tcg)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

# Allow obtaining code coverage data
option(CODE_COVERAGE "Enable coverage reporting" OFF)

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
)

find_package(Boost 1.71.0 COMPONENTS filesystem REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Boost_FILESYSTEM_LIBRARY} fmt::fmt spdlog::spdlog)

target_compile_options(${PROJECT_NAME} PUBLIC -Werror -pedantic)

configure_file(src/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# gcda is the file generated at runtime
# gcno is the file generated at compile time
#
# See:
# https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html
# https://gcc.gnu.org/onlinedocs/gcc-10.1.0/gcc/Instrumentation-Options.html
option(GCDA_DIR "Argument for -fprofile-dir=")

if(CODE_COVERAGE)
  set(BUILD_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}.dir/src")
  set(CODECOV_CXX_FLAGS --coverage -fprofile-prefix-path=${BUILD_SRC_DIR})

  target_compile_options(${PROJECT_NAME} PUBLIC ${CODECOV_CXX_FLAGS})
  target_link_options(${PROJECT_NAME} PUBLIC ${CODECOV_CXX_FLAGS})
  if(NOT GCDA_DIR STREQUAL "")
    target_compile_options(${PROJECT_NAME} PUBLIC -fprofile-dir=${GCDA_DIR})
    target_link_options(${PROJECT_NAME} PUBLIC -fprofile-dir=${GCDA_DIR})
  endif()
endif()
