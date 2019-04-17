cmake_minimum_required(VERSION 3.5)
project(program_qt)

#set(QT_HOME "/home/allen/Programs/Qt/5.8/gcc_64")     #对应的Qt的安装目录
#set(CMAKE_PREFIX_PATH ${QT_HOME})

#SET(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
#set(CMAKE_INCLUDE_CURRENT_DIR on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTOUIC on)
set(CMAKE_AUTORCC on)
###这里根据Project的实际情况来添加需要的模块
find_package(Qt5 REQUIRED Core Widgets UiTools Xml Network Qml Quick)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp mainwindow.h mainwindow.cpp Mylabel.cpp Mylabel.h camera.h camera.cpp
        serial.h serial.cpp common.h common.cpp test.cpp bmpfile.h)
#这里需要注意，如果你的Projetc中包含了qrc文件，则必须要将该qrc文件也加进来
add_executable(program_qt ${SOURCE_FILES})
target_link_libraries(program_qt ${OpenCV_LIBS} JHCap Qt5::Core Qt5::Widgets Qt5::Gui Qt5::UiTools Qt5::Xml Qt5::Network Qt5::Qml Qt5::Quick)