# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_COMMAND = /home/app/clion-2017.1.4/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/app/clion-2017.1.4/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/app/CLionProjects/systemCheck

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/app/CLionProjects/systemCheck/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/systemCheck.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/systemCheck.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/systemCheck.dir/flags.make

CMakeFiles/systemCheck.dir/main.cpp.o: CMakeFiles/systemCheck.dir/flags.make
CMakeFiles/systemCheck.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/app/CLionProjects/systemCheck/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/systemCheck.dir/main.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/systemCheck.dir/main.cpp.o -c /home/app/CLionProjects/systemCheck/main.cpp

CMakeFiles/systemCheck.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/systemCheck.dir/main.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/app/CLionProjects/systemCheck/main.cpp > CMakeFiles/systemCheck.dir/main.cpp.i

CMakeFiles/systemCheck.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/systemCheck.dir/main.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/app/CLionProjects/systemCheck/main.cpp -o CMakeFiles/systemCheck.dir/main.cpp.s

CMakeFiles/systemCheck.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/systemCheck.dir/main.cpp.o.requires

CMakeFiles/systemCheck.dir/main.cpp.o.provides: CMakeFiles/systemCheck.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/systemCheck.dir/build.make CMakeFiles/systemCheck.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/systemCheck.dir/main.cpp.o.provides

CMakeFiles/systemCheck.dir/main.cpp.o.provides.build: CMakeFiles/systemCheck.dir/main.cpp.o


# Object files for target systemCheck
systemCheck_OBJECTS = \
"CMakeFiles/systemCheck.dir/main.cpp.o"

# External object files for target systemCheck
systemCheck_EXTERNAL_OBJECTS =

systemCheck: CMakeFiles/systemCheck.dir/main.cpp.o
systemCheck: CMakeFiles/systemCheck.dir/build.make
systemCheck: CMakeFiles/systemCheck.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/app/CLionProjects/systemCheck/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable systemCheck"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/systemCheck.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/systemCheck.dir/build: systemCheck

.PHONY : CMakeFiles/systemCheck.dir/build

CMakeFiles/systemCheck.dir/requires: CMakeFiles/systemCheck.dir/main.cpp.o.requires

.PHONY : CMakeFiles/systemCheck.dir/requires

CMakeFiles/systemCheck.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/systemCheck.dir/cmake_clean.cmake
.PHONY : CMakeFiles/systemCheck.dir/clean

CMakeFiles/systemCheck.dir/depend:
	cd /home/app/CLionProjects/systemCheck/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/app/CLionProjects/systemCheck /home/app/CLionProjects/systemCheck /home/app/CLionProjects/systemCheck/cmake-build-debug /home/app/CLionProjects/systemCheck/cmake-build-debug /home/app/CLionProjects/systemCheck/cmake-build-debug/CMakeFiles/systemCheck.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/systemCheck.dir/depend

