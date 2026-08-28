cmake_minimum_required(VERSION 3.21)
project(CislunarSim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

add_executable(CislunarSim
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/renderer.h
    src/renderer.cpp
    src/physics.h
    src/physics.cpp
)

target_link_libraries(CislunarSim PRIVATE
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

if(WIN32)
    set_target_properties(CislunarSim PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
