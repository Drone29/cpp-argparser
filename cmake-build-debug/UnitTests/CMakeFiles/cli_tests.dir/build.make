# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/clion/121/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/121/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/andrey/CLionProjects/argparser

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/andrey/CLionProjects/argparser/cmake-build-debug

# Include any dependencies generated for this target.
include UnitTests/CMakeFiles/cli_tests.dir/depend.make

# Include the progress variables for this target.
include UnitTests/CMakeFiles/cli_tests.dir/progress.make

# Include the compile flags for this target's objects.
include UnitTests/CMakeFiles/cli_tests.dir/flags.make

UnitTests/CMakeFiles/cli_tests.dir/cli_tests.cpp.o: UnitTests/CMakeFiles/cli_tests.dir/flags.make
UnitTests/CMakeFiles/cli_tests.dir/cli_tests.cpp.o: ../UnitTests/cli_tests.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/andrey/CLionProjects/argparser/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object UnitTests/CMakeFiles/cli_tests.dir/cli_tests.cpp.o"
	cd /home/andrey/CLionProjects/argparser/cmake-build-debug/UnitTests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cli_tests.dir/cli_tests.cpp.o -c /home/andrey/CLionProjects/argparser/UnitTests/cli_tests.cpp

UnitTests/CMakeFiles/cli_tests.dir/cli_tests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cli_tests.dir/cli_tests.cpp.i"
	cd /home/andrey/CLionProjects/argparser/cmake-build-debug/UnitTests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/andrey/CLionProjects/argparser/UnitTests/cli_tests.cpp > CMakeFiles/cli_tests.dir/cli_tests.cpp.i

UnitTests/CMakeFiles/cli_tests.dir/cli_tests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cli_tests.dir/cli_tests.cpp.s"
	cd /home/andrey/CLionProjects/argparser/cmake-build-debug/UnitTests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/andrey/CLionProjects/argparser/UnitTests/cli_tests.cpp -o CMakeFiles/cli_tests.dir/cli_tests.cpp.s

# Object files for target cli_tests
cli_tests_OBJECTS = \
"CMakeFiles/cli_tests.dir/cli_tests.cpp.o"

# External object files for target cli_tests
cli_tests_EXTERNAL_OBJECTS =

UnitTests/cli_tests: UnitTests/CMakeFiles/cli_tests.dir/cli_tests.cpp.o
UnitTests/cli_tests: UnitTests/CMakeFiles/cli_tests.dir/build.make
UnitTests/cli_tests: UnitTests/CMakeFiles/cli_tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/andrey/CLionProjects/argparser/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable cli_tests"
	cd /home/andrey/CLionProjects/argparser/cmake-build-debug/UnitTests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cli_tests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
UnitTests/CMakeFiles/cli_tests.dir/build: UnitTests/cli_tests

.PHONY : UnitTests/CMakeFiles/cli_tests.dir/build

UnitTests/CMakeFiles/cli_tests.dir/clean:
	cd /home/andrey/CLionProjects/argparser/cmake-build-debug/UnitTests && $(CMAKE_COMMAND) -P CMakeFiles/cli_tests.dir/cmake_clean.cmake
.PHONY : UnitTests/CMakeFiles/cli_tests.dir/clean

UnitTests/CMakeFiles/cli_tests.dir/depend:
	cd /home/andrey/CLionProjects/argparser/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/andrey/CLionProjects/argparser /home/andrey/CLionProjects/argparser/UnitTests /home/andrey/CLionProjects/argparser/cmake-build-debug /home/andrey/CLionProjects/argparser/cmake-build-debug/UnitTests /home/andrey/CLionProjects/argparser/cmake-build-debug/UnitTests/CMakeFiles/cli_tests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : UnitTests/CMakeFiles/cli_tests.dir/depend
