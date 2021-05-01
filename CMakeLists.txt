cmake_minimum_required(VERSION 3.16)
include(cmake/libseccomp.cmake)
include(cmake/version.cmake)

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
    src/sandbox.cpp
)

find_package(Boost 1.71.0 COMPONENTS filesystem REQUIRED)
find_package(fmt 7 REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(${PROJECT_NAME} ${Boost_FILESYSTEM_LIBRARY} fmt::fmt spdlog::spdlog)
if(USE_SECCOMP)
  target_link_libraries(${PROJECT_NAME} seccomp)
endif()

target_compile_options(${PROJECT_NAME} PUBLIC -Werror -pedantic)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")

configure_file(src/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS tcg
       RUNTIME DESTINATION bin
       PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
                   GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
)

include(cmake/coverage.cmake)
include(cmake/cpack.cmake)
