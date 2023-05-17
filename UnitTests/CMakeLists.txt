
set(TEST_NAME "argparser_test")
project(${TEST_NAME})
# include directory with files to be tested
include_directories(../)
add_executable(internal_tests internal_tests.cpp)
add_executable(cli_tests cli_tests.cpp)

add_test(NAME internal_tests COMMAND internal_tests)

add_test(NAME cli_test_1 COMMAND cli_tests -iii -jjj)
add_test(NAME cli_test_2 COMMAND cli_tests -iji)
add_test(NAME cli_test_3 COMMAND cli_tests -p 123)
add_test(NAME cli_test_4 COMMAND cli_tests -p123)
add_test(NAME cli_test_5 COMMAND cli_tests -p=123)
add_test(NAME cli_test_6 COMMAND cli_tests -p -var 1 2 3)
add_test(NAME cli_test_7 COMMAND cli_tests -p=-var -var 1 2 3)
