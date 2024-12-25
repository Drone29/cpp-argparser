cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME "argparser")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} example.cpp)

enable_testing()

# add subdir with tests
add_subdirectory(utest)