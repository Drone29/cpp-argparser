cmake_minimum_required(VERSION 3.16)
set(TEST_NAME "argparser_test")
set(CMAKE_CXX_STANDARD 17)
project(${TEST_NAME})
# include directory with files to be tested
include_directories(../)
add_executable(internal_tests internal_tests.cpp)

add_test(NAME internal_tests COMMAND internal_tests)