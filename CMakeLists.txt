cmake_minimum_required(VERSION 3.20)
project(circuit_simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/eigen-3.4.0)

add_executable(circuit_simulator main.cpp
    Circuit.cpp
    Component.cpp
    Simulator.cpp)
