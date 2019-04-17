cmake_minimum_required(VERSION 3.5)
project(start)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES update.cpp)
add_executable(start ${SOURCE_FILES})
target_link_libraries(start)
