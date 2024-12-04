#[[
SPDX-FileCopyrightText: 2024 basysKom GmbH
SPDX-FileContributor: Berthold Krevert <berthold.krevert@basyskom.com>
SPDX-License-Identifier: BSD-3-Clause
]]

cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME "QmlMobilenet")
project(${PROJECT_NAME})

add_subdirectory(3rdparty)
#add_subdirectory(minimal)

find_package(Qt5Core         REQUIRED)
find_package(Qt5Gui          REQUIRED)
find_package(Qt5Qml          REQUIRED)
find_package(Qt5Quick        REQUIRED)
find_package(Qt5Multimedia   REQUIRED)

add_subdirectory(src)

