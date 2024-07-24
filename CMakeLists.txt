cmake_minimum_required(VERSION 3.5)

project(Irken VERSION 6.12.4 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#использую 17й стандарт для корректной работы смарт поинтеров без использования make_unique[_shared]<>
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Core Network Test Charts REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Network Test Charts REQUIRED)

include_directories(
    src
    src/telegramAlert
    src/klines
    src/base
    src/spredCalculator
    )

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/settings.h
        src/instrumentsinfo.cpp
        src/instrumentsinfo.h

        src/klines/candlestickwidget.h
        src/klines/candlestickwidget.cpp
        src/klines/klinesworkingspace.h
        src/klines/klinesworkingspace.cpp

        src/base/methods.h
        src/base/methods.cpp
        src/base/requests.h
        src/base/requests.cpp

        src/spredCalculator/spredcalculatorwgt.cpp
        src/spredCalculator/spredcalculatorwgt.h

        src/telegramAlert/telegramalert.h
        src/telegramAlert/telegramalert.cpp

        README.txt
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Irken
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Irken APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Irken SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Irken
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Irken
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Charts
    Threads::Threads)
set_target_properties(Irken PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Irken)
endif()
