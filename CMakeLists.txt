cmake_minimum_required(VERSION 3.16)

project(ChatAppQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ----------- TODOS los archivos fuente -----------
set(PROJECT_SOURCES
    main.cpp

    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    chatcore.cpp
    chatcore.h

    logindialog.cpp
    logindialog.h
    logindialog.ui

    registerdialog.cpp
    registerdialog.h
    registerdialog.ui

    usersettingsdialog.cpp
    usersettingsdialog.h
    usersettingsdialog.ui

    resetpassworddialog.cpp
    resetpassworddialog.h
    resetpassworddialog.ui
)

# ----------- Ejecutable -----------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ChatAppQt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
        usersettingsdialog.h usersettingsdialog.cpp usersettingsdialog.ui
        resetpassworddialog.h resetpassworddialog.cpp resetpassworddialog.ui
    )
else()
    add_executable(ChatAppQt
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(ChatAppQt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# ----------- Qt 6 finalización -----------
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ChatAppQt)
endif()
