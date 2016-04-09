find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

set(gui_UI_FILES
  boomerang.ui
  about.ui
  LoggingSettingsDlg.ui
)
QT5_WRAP_UI(gui_UI_H ${gui_UI_FILES})

SET(boomerang_SRC
    main.cpp
    mainwindow
    rtleditor
    mypthread
    commandlinedriver
    DecompilerThread
    LoggingSettingsDlg
)
qt5_add_resources(resources_SRC boomerang.qrc)
ADD_EXECUTABLE(boomerang ${boomerang_SRC} ${gui_UI_H} ${resources_SRC})
TARGET_LINK_LIBRARIES(boomerang
${GC_LIBS}
${DEBUG_LIB}
boom_base frontend db type boomerang_DSLs codegen util boom_base
boomerang_passes
)
if (NOT WIN32)
TARGET_LINK_LIBRARIES(boomerang pthread)
endif()
qt5_use_modules(boomerang Core Xml Widgets)
