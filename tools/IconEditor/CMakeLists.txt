cmake_minimum_required(VERSION 3.16)
project(PocketMageIconEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(IconEditor
    main.cpp
    mainwindow.cpp
    mainwindow.h
    pixelcanvas.cpp
    pixelcanvas.h
)

target_link_libraries(IconEditor PRIVATE Qt6::Widgets)

# macOS bundle settings
if(APPLE)
    set_target_properties(IconEditor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pocketmage.iconeditor"
        MACOSX_BUNDLE_BUNDLE_NAME "PocketMage Icon Editor"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()
