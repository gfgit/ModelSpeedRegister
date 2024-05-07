cmake_minimum_required(VERSION 3.5)

project(ModelSpeedRegister VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        commandstation/icommandstation.h
        commandstation/icommandstation.cpp
        commandstation/utils.h

        commandstation/dummycommandstation.h
        commandstation/dummycommandstation.cpp

        input/ispeedsensor.h
        input/ispeedsensor.cpp

        input/dummyspeedsensor.h
        input/dummyspeedsensor.cpp

        recorder/recordingmanager.h
        recorder/recordingmanager.cpp
        recorder/locomotiverecording.h
        recorder/locomotiverecording.cpp

        view/locomotiverecordingview.h
        view/locomotiverecordingview.cpp

        chart/chartview.h
        chart/chartview.cpp
        chart/chart.h
        chart/chart.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ModelSpeedRegister
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        commandstation/icommandstation.h commandstation/icommandstation.cpp
        input/ispeedsensor.h input/ispeedsensor.cpp
        recorder/recordingmanager.h recorder/recordingmanager.cpp
        recorder/locomotiverecording.h recorder/locomotiverecording.cpp
        view/locomotiverecordingview.h view/locomotiverecordingview.cpp
        commandstation/dummycommandstation.h commandstation/dummycommandstation.cpp
        input/dummyspeedsensor.h input/dummyspeedsensor.cpp
        chart/chartview.h chart/chartview.cpp
        chart/chart.h chart/chart.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ModelSpeedRegister APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ModelSpeedRegister SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ModelSpeedRegister
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ModelSpeedRegister PRIVATE
    Qt6::Widgets
    Qt6::Charts
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ModelSpeedRegister)
endif()
set_target_properties(ModelSpeedRegister PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ModelSpeedRegister
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ModelSpeedRegister)
endif()
