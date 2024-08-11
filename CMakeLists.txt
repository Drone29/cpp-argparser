cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME "argparser")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} example.cpp)

# specify include dir for tests
set(TEST_INC_DIR ${PROJECT_SOURCE_DIR})
# tell utest CMakeLists that there are no test sources
set(NO_TEST_SRC 1)

enable_testing()

# add subdir with tests
add_subdirectory(${PROJECT_SOURCE_DIR}/utest)