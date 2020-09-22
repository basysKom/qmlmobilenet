set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11 -Wno-deprecated-declarations -Wno-unused-variable")

qt5_add_resources(QT_RESOURCES qml.qrc)
add_executable(${PROJECT_NAME}
    main.cpp
    cocodetectionfilter.cpp cocodetectionfilter.h
    ${QT_RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Multimedia
)

install(TARGETS ${PROJECT_NAME} DESTINATION /bin)
