set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11 -Wno-deprecated-declarations -Wno-unused-variable")

if (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif (UNIX)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/tensorflow ${CMAKE_BINARY_DIR}/flatbuffers/include)

qt5_add_resources(QT_RESOURCES qml.qrc)
add_executable(${PROJECT_NAME}
    main.cpp
    cocodetectionfilter.cpp cocodetectionfilter.h
    ${QT_RESOURCES}
)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Qml
        Qt5::Quick
        Qt5::Multimedia
        ${CMAKE_BINARY_DIR}/3rdparty/tensorflow/tensorflow/lite/libtensorflowlite.a
        dl
    )
else (UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Qml
        Qt5::Quick
        Qt5::Multimedia
        ${CMAKE_BINARY_DIR}/3rdparty/tensorflow/tensorflow/lite/libtensorflowlite.a
    )
endif (UNIX)

install(TARGETS ${PROJECT_NAME} DESTINATION /bin)
