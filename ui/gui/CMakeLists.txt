find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

set(gui_UI_FILES
  boomerang.ui
  about.ui
)
QT5_WRAP_UI(gui_UI_H ${gui_UI_FILES})

SET(boomerang_gui_SRC
    main.cpp
    mainwindow
    rtleditor
    mypthread
    DecompilerThread
)
qt5_add_resources(resources_SRC boomerang.qrc)
ADD_EXECUTABLE(boomerang_gui ${boomerang_gui_SRC} ${gui_UI_H} ${resources_SRC})
TARGET_LINK_LIBRARIES(boomerang_gui
${DL_LIBS}
${GC_LIBS}
${DEBUG_LIB}
${EXPAT_LIBRARIES}
boom_base frontend db type boomerang_DSLs codegen util BinaryFile boom_base
pthread
)
qt5_use_modules(boomerang_gui Core Xml Widgets)
