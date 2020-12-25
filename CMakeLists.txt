project(tcg)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


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


find_package(Boost COMPONENTS filesystem REQUIRED)
add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} ${Boost_FILESYSTEM_LIBRARY} fmt::fmt)
