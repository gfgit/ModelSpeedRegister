cmake_minimum_required(VERSION 3.5)

project(ModelSpeedRegister VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Charts Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Network)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ModelSpeedRegister
        MANUAL_FINALIZATION

        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        chart/chartview.h chart/chartview.cpp
        chart/chart.h chart/chart.cpp

        commandstation/backends/endian.h
        commandstation/backends/z21messages.h
        commandstation/backends/z21commandstation.h
        commandstation/backends/z21commandstation.cpp

        commandstation/dummycommandstation.h commandstation/dummycommandstation.cpp
        commandstation/icommandstation.h commandstation/icommandstation.cpp

        input/dummyspeedsensor.h input/dummyspeedsensor.cpp
        input/espanaloghallsensor.h input/espanaloghallsensor.cpp
        input/espanaloghallconfigwidget.h input/espanaloghallconfigwidget.cpp
        input/ispeedsensor.h input/ispeedsensor.cpp

        recorder/recordingmanager.h recorder/recordingmanager.cpp
        recorder/locoinfo.h recorder/locoinfo.cpp
        recorder/idataseries.h recorder/idataseries.cpp
        recorder/series/requestedspeedstepseries.h recorder/series/requestedspeedstepseries.cpp
        recorder/series/receivedspeedstepseries.cpp
        recorder/series/receivedspeedstepseries.h
        recorder/series/movingaverageseries.h recorder/series/movingaverageseries.cpp
        recorder/series/rawsensordataseries.h recorder/series/rawsensordataseries.cpp
        recorder/series/sensortravelleddistanceseries.h recorder/series/sensortravelleddistanceseries.cpp
        recorder/series/totalstepaverageseries.h recorder/series/totalstepaverageseries.cpp
        recorder/series/dataseriescurvemapping.h recorder/series/dataseriescurvemapping.cpp

        train/train.h train/train.cpp
        train/locomotive.h train/locomotive.cpp
        train/locomotivepool.h train/locomotivepool.cpp
        train/locospeedmapping.h train/locospeedmapping.cpp
        train/trainspeedtable.h train/trainspeedtable.cpp
        train/locostatuswidget.h train/locostatuswidget.cpp

        view/dataseriesfiltermodel.h view/dataseriesfiltermodel.cpp
        view/dataseriesgraph.h view/dataseriesgraph.cpp
        view/speedcurvetablemodel.h view/speedcurvetablemodel.cpp
        view/locomotiverecordingview.h view/locomotiverecordingview.cpp
        view/locospeedcurveview.h view/locospeedcurveview.cpp
        view/starttestdlg.h view/starttestdlg.cpp
        recorder/rawspeedcurveio.h recorder/rawspeedcurveio.cpp
        view/traintab.h view/traintab.cpp
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
    Qt6::Network
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
