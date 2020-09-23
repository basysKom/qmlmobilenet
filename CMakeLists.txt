cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME "QmlDetector")
project(${PROJECT_NAME})

add_subdirectory(3rdparty)

find_package(Qt5Core         REQUIRED)
find_package(Qt5Gui          REQUIRED)
find_package(Qt5Qml          REQUIRED)
find_package(Qt5Quick        REQUIRED)
find_package(Qt5Multimedia   REQUIRED)

add_subdirectory(src)

