project(tcg)

find_package(fmt REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)

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