cmake_minimum_required(VERSION 3.16)
project(DeadlineSaver VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp      mainwindow.h
    tugaspage.cpp       tugaspage.h
    jadwalpage.cpp      jadwalpage.h
    datamanager.cpp     datamanager.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DeadlineSaver MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(DeadlineSaver ${PROJECT_SOURCES})
endif()

target_link_libraries(DeadlineSaver PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)

set_target_properties(DeadlineSaver PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DeadlineSaver)
endif()
