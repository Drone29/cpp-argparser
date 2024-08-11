# set utest exe name
set(TEST_EXE "utest")
set(TEST_NAME "UnitTests")
# use fetch content
include(FetchContent)
# declare content (where to download/extract at configure time)
FetchContent_Declare(
        googletest
        URL "https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip"
        DOWNLOAD_EXTRACT_TIMESTAMP FALSE # suppress warning
)
# make available
FetchContent_MakeAvailable(googletest)

# check for required paths/files
if(NOT TEST_INC_DIR)
    message(FATAL_ERROR "TEST_INC_DIR is not defined!")
endif()
if(NOT NO_TEST_SRC AND NOT TEST_SRC)
    message(FATAL_ERROR "TEST_SRC is not defined!")
endif()    

# gather all source files in the current dir
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} LOCAL_SRC)

# add test executable (with inherited sources list)
add_executable(${TEST_EXE} ${LOCAL_SRC} ${TEST_SRC})
# include .h files location
target_include_directories(${TEST_EXE} PRIVATE ${TEST_INC_DIR})
# link gtest libs to test exe
target_link_libraries(
    ${TEST_EXE}
    GTest::gtest_main
#    GTest::gmock_main
)

# discover test within test exe
include(GoogleTest)
#gtest_discover_tests(${TEST_EXE})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_EXE} --gtest_color=yes)