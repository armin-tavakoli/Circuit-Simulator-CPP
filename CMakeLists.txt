cmake_minimum_required(VERSION 3.20)
project(circuit_simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(${CMAKE_SOURCE_DIR}/eigen-3.4.0)

add_executable(circuit_simulator
        # فایل‌های اصلی
        main.cpp
        mainwindow.h
        mainwindow.cpp

        # فایل‌های ویرایشگر
        schematiceditor.h
        schematiceditor.cpp

        # فایل‌های آیتم‌های گرافیکی
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

        # فایل‌های بک‌اند
        Circuit.h
        Circuit.cpp
        Component.h
        Component.cpp
        Simulator.h
        Simulator.cpp
        DiodeModel.h
        PrintRequest.h
        ValueParser.h
)

target_link_libraries(circuit_simulator PRIVATE Qt6::Widgets)
