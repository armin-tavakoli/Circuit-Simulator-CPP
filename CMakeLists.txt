cmake_minimum_required(VERSION 3.20)
project(circuit_simulator)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

include_directories(${CMAKE_SOURCE_DIR}/eigen-3.4.0)
include_directories(${CMAKE_SOURCE_DIR}/libs)


add_executable(circuit_simulator
        main.cpp
        mainwindow.h
        mainwindow.cpp
        schematiceditor.h
        schematiceditor.cpp
        componentitem.h
        componentitem.cpp
        terminalitem.h
        terminalitem.cpp
        grounditem.h
        grounditem.cpp
        resistoritem.h
        resistoritem.cpp
        capacitoritem.h
        capacitoritem.cpp
        inductoritem.h
        inductoritem.cpp
        voltagesourceitem.h
        voltagesourceitem.cpp
        currentsourceitem.h
        currentsourceitem.cpp
        dependentsourceitems.h
        dependentsourceitems.cpp
        polylinewireitem.h
        polylinewireitem.cpp
        junctionitem.h
        scopewindow.h
        scopewindow.cpp
        Circuit.h
        Circuit.cpp
        Component.h
        Component.cpp
        Simulator.h
        Simulator.cpp
        DiodeModel.h
        PrintRequest.h
        ValueParser.h
        propertiesdialog.cpp
        propertiesdialog.h
        cereal_registration.h
        transientdialog.h
        transientdialog.cpp
)

target_link_libraries(circuit_simulator PRIVATE Qt6::Widgets Qt6::Charts)
