#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets Gui Xml REQUIRED HINTS $ENV{QTDIR})
if (Qt5_FOUND)
    mark_as_advanced(Qt5_DIR Qt5Widgets_DIR Qt5Gui_DIR Qt5Xml_DIR)
endif (Qt5_FOUND)

set(gui_UI_FILES
    About.ui
    SettingsDlg.ui
    MainWindow.ui
)

QT5_WRAP_UI(gui_UI_H ${gui_UI_FILES})

SET(boomerang_SRC
    Decompiler.cpp
    Decompiler.h
    SettingsDlg.cpp
    SettingsDlg.h
    Main.cpp
    MainWindow.cpp
    MainWindow.h
    RTLEditor.cpp
    RTLEditor.h
)

qt5_add_resources(resources_SRC Boomerang.qrc)
add_executable(boomerang-gui ${boomerang_SRC} ${gui_UI_H} ${resources_SRC})

set_property(SOURCE
        "${CMAKE_CURRENT_BINARY_DIR}/ui_About.h"
        "${CMAKE_CURRENT_BINARY_DIR}/ui_SettingsDlg.h"
        "${CMAKE_CURRENT_BINARY_DIR}/ui_MainWindow.h"
        "${CMAKE_CURRENT_BINARY_DIR}/qrc_Boomerang.cpp"
    PROPERTY SKIP_AUTOMOC ON
)

target_link_libraries(boomerang-gui
    boomerang
    ${GC_LIBS}
    ${DEBUG_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
    Qt5::Core
    Qt5::Xml
    Qt5::Widgets
)

# installation
install(TARGETS boomerang-gui
    RUNTIME DESTINATION bin/
)

if (WIN32 AND NOT UNIX)
    include(boomerang-windeployqt)
    BOOMERANG_WINDEPLOYQT(boomerang-gui bin/)
endif ()
