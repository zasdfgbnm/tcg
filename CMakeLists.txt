project(tcg)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt REQUIRED PATHS third_party/fmt NO_DEFAULT_PATH)
if(fmt_FOUND)
    message ("fmt found")
else()
    message (FATAL_ERROR "Cannot find fmt")
endif()

find_package(Boost COMPONENTS filesystem REQUIRED)
if(Boost_FOUND)
    message ("Boost found")
else()
    message (FATAL_ERROR "Cannot find Boost")
endif()



add_executable(${PROJECT_NAME}
    src/main.cpp
    src/utils.cpp
    src/utils.hpp
    src/create.cpp
    src/freeze.cpp
    src/list.cpp
    src/help.cpp
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME}
    fmt::fmt
    ${Boost_FILESYSTEM_LIBRARY}
)

message(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
message(FMT_INCLUDE_DIRS ${Fmt_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${FMT_INCLUDE_DIRS}
)