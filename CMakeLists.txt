cmake_minimum_required(VERSION 3.5)
project(program_1202)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main_new.cpp rect_image.cpp rect_image.h match.h match.cpp serial.h serial.cpp crc32.h lbp.h common.h)
add_executable(program_1202 ${SOURCE_FILES})

target_link_libraries(program_1202 ${OpenCV_LIBS} JHCap)
