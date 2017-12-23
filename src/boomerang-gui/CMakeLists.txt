#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED)
if (Qt5Widgets_FOUND)
    mark_as_advanced(Qt5Widgets_DIR Qt5Gui_DIR)
endif(Qt5Widgets_FOUND)


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

target_link_libraries(boomerang-gui
    boomerang
    ${GC_LIBS}
    ${DEBUG_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
    Qt5::Core
    Qt5::Xml
    Qt5::Widgets
)

include(boomerang-utils)
BOOMERANG_COPY_IMPORTED_DLL(boomerang-gui Qt5::Core)
BOOMERANG_COPY_IMPORTED_DLL(boomerang-gui Qt5::Widgets)
BOOMERANG_COPY_IMPORTED_DLL(boomerang-gui Qt5::Gui)

# installation
install(TARGETS boomerang-gui
    RUNTIME DESTINATION bin/
)
