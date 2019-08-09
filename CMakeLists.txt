cmake_minimum_required(VERSION 3.5)
project(program)

set(CMAKE_AUTOMOC on)
set(CMAKE_AUTOUIC on)
set(CMAKE_AUTORCC on)
###这里根据Project的实际情况来添加需要的模块
find_package(Qt5 REQUIRED Core Widgets UiTools Xml Network Qml Quick)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp mainwindow.cpp mainwindow.h mythread.cpp mythread.h capture.h capture.cpp
        Mylabel.cpp Mylabel.h camera.h camera.cpp serial.h serial.cpp common.h common.cpp test.cpp bmpfile.h)
add_executable(program ${SOURCE_FILES})
target_link_libraries(program Threads::Threads ${OpenCV_LIBS} JHCap Qt5::Core Qt5::Widgets Qt5::Gui Qt5::UiTools Qt5::Xml Qt5::Network Qt5::Qml Qt5::Quick)
