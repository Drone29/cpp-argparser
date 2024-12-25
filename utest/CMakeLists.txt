# set utest exe name
set(TEST_EXE "utest")
set(TEST_NAME "UnitTests")
# use fetch content
include(FetchContent)
# declare content (where to download/extract at configure time)
FetchContent_Declare(
        googletest
        URL "https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip"
#        DOWNLOAD_EXTRACT_TIMESTAMP FALSE # suppress warning
)
# make available
FetchContent_MakeAvailable(googletest)

# gather all source files in the current dir
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} LOCAL_SRC)

# add test executable (with inherited sources list)
add_executable(${TEST_EXE} utests.cpp utests_internal.cpp)
target_include_directories(${TEST_EXE} PUBLIC ${CMAKE_SOURCE_DIR})
# link gtest libs to test exe
target_link_libraries(
    ${TEST_EXE}
    GTest::gtest_main
#    GTest::gmock_main
)

# discover test within test exe
include(GoogleTest)
set(GTEST_COLOR yes)
gtest_discover_tests(${TEST_EXE} EXTRA_ARGS --gtest_color=yes)