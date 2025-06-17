cmake_minimum_required(VERSION 3.20)
project(circuit_simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/eigen-3.4.0)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Widgets Charts REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(circuit_simulator main.cpp
    Circuit.cpp
    Component.cpp
    Simulator.cpp
    MainWindow.cpp
    CircuitView.cpp
    TransientAnalysisDialog.cpp
    PlotWindow.cpp
    resources.qrc)

target_link_libraries(circuit_simulator PRIVATE Qt6::Widgets)
target_link_libraries(circuit_simulator PRIVATE Qt6::Widgets Qt6::Charts)