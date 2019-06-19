cmake_minimum_required (VERSION 2.6)
project (autocgrouper)
set (CMAKE_CXX_STANDARD 11)
add_executable(autocgrouperd src/autocgrouperd.cpp src/settings.cpp)