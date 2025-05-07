cmake_minimum_required(VERSION 3.16)

project(Inkscape  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Inkscape 
    include/MainWindow.h
    include/CanvasWidget.h
    include/BrushWidthSpinBox.h
    include/ToolBar.h
    include/Shapes/Shape.h
    include/Shapes/LineShape.h
    include/Shapes/CircleShape.h
    include/Shapes/FreehandShape.h
    include/Shapes/RectangleShape.h
    include/Shapes/PolygonShape.h
    include/Shapes/RegularPolygonShape.h
    src/main.cpp
    src/MainWindow.cpp
    src/CanvasWidget.cpp 
    src/ToolBar.cpp 
    src/Shapes/LineShape.cpp
    src/Shapes/CircleShape.cpp
    src/Shapes/RectangleShape.cpp
    src/Shapes/FreehandShape.cpp
    src/Shapes/PolygonShape.cpp
    src/Shapes/RegularPolygonShape.cpp
    resources/resources.qrc 
)

target_link_libraries(Inkscape  PRIVATE Qt6::Widgets)