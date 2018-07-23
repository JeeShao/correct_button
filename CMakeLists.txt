cmake_minimum_required(VERSION 3.5)
project(program)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp rect_image.cpp rect_image.h match.h match.cpp serial.h serial.cpp crc32.h)
add_executable(program ${SOURCE_FILES})

target_link_libraries(program ${OpenCV_LIBS})