cmake_minimum_required(VERSION 3.5)
project(program2019)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp rect_image.h rect_image.cpp match.h match.cpp serial.h serial.cpp crc32.h common.h capture.h capture.cpp camera.h camera.cpp)
add_executable(program2019 ${SOURCE_FILES})

target_link_libraries(program2019 ${OpenCV_LIBS} JHCap)
