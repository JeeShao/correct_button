# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/app/update

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/app/update

# Include any dependencies generated for this target.
include CMakeFiles/update.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/update.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/update.dir/flags.make

CMakeFiles/update.dir/update.cpp.o: CMakeFiles/update.dir/flags.make
CMakeFiles/update.dir/update.cpp.o: update.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/app/update/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/update.dir/update.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/update.dir/update.cpp.o -c /home/app/update/update.cpp

CMakeFiles/update.dir/update.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/update.dir/update.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/app/update/update.cpp > CMakeFiles/update.dir/update.cpp.i

CMakeFiles/update.dir/update.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/update.dir/update.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/app/update/update.cpp -o CMakeFiles/update.dir/update.cpp.s

CMakeFiles/update.dir/update.cpp.o.requires:

.PHONY : CMakeFiles/update.dir/update.cpp.o.requires

CMakeFiles/update.dir/update.cpp.o.provides: CMakeFiles/update.dir/update.cpp.o.requires
	$(MAKE) -f CMakeFiles/update.dir/build.make CMakeFiles/update.dir/update.cpp.o.provides.build
.PHONY : CMakeFiles/update.dir/update.cpp.o.provides

CMakeFiles/update.dir/update.cpp.o.provides.build: CMakeFiles/update.dir/update.cpp.o


# Object files for target update
update_OBJECTS = \
"CMakeFiles/update.dir/update.cpp.o"

# External object files for target update
update_EXTERNAL_OBJECTS =

update: CMakeFiles/update.dir/update.cpp.o
update: CMakeFiles/update.dir/build.make
update: CMakeFiles/update.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/app/update/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable update"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/update.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/update.dir/build: update

.PHONY : CMakeFiles/update.dir/build

CMakeFiles/update.dir/requires: CMakeFiles/update.dir/update.cpp.o.requires

.PHONY : CMakeFiles/update.dir/requires

CMakeFiles/update.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/update.dir/cmake_clean.cmake
.PHONY : CMakeFiles/update.dir/clean

CMakeFiles/update.dir/depend:
	cd /home/app/update && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/app/update /home/app/update /home/app/update /home/app/update /home/app/update/CMakeFiles/update.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/update.dir/depend

