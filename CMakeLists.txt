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
    src/utils.hpp
    src/command.cpp
    src/command.hpp
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
