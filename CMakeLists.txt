cmake_minimum_required(VERSION 3.16)
project(argparser)

set(CMAKE_CXX_STANDARD 17)

add_executable(argparser main.cpp)

enable_testing()

include(CTest) # We include CTest which is part of CMake
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(UnitTests)
endif ()