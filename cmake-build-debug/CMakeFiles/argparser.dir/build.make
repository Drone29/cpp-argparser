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
include CMakeFiles/argparser.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/argparser.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/argparser.dir/flags.make

CMakeFiles/argparser.dir/example.cpp.o: CMakeFiles/argparser.dir/flags.make
CMakeFiles/argparser.dir/example.cpp.o: ../example.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/andrey/CLionProjects/argparser/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/argparser.dir/example.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/argparser.dir/example.cpp.o -c /home/andrey/CLionProjects/argparser/example.cpp

CMakeFiles/argparser.dir/example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/argparser.dir/example.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/andrey/CLionProjects/argparser/example.cpp > CMakeFiles/argparser.dir/example.cpp.i

CMakeFiles/argparser.dir/example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/argparser.dir/example.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/andrey/CLionProjects/argparser/example.cpp -o CMakeFiles/argparser.dir/example.cpp.s

# Object files for target argparser
argparser_OBJECTS = \
"CMakeFiles/argparser.dir/example.cpp.o"

# External object files for target argparser
argparser_EXTERNAL_OBJECTS =

argparser: CMakeFiles/argparser.dir/example.cpp.o
argparser: CMakeFiles/argparser.dir/build.make
argparser: CMakeFiles/argparser.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/andrey/CLionProjects/argparser/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable argparser"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/argparser.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/argparser.dir/build: argparser

.PHONY : CMakeFiles/argparser.dir/build

CMakeFiles/argparser.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/argparser.dir/cmake_clean.cmake
.PHONY : CMakeFiles/argparser.dir/clean

CMakeFiles/argparser.dir/depend:
	cd /home/andrey/CLionProjects/argparser/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/andrey/CLionProjects/argparser /home/andrey/CLionProjects/argparser /home/andrey/CLionProjects/argparser/cmake-build-debug /home/andrey/CLionProjects/argparser/cmake-build-debug /home/andrey/CLionProjects/argparser/cmake-build-debug/CMakeFiles/argparser.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/argparser.dir/depend

