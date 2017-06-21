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
    commandlinedriver
    DecompilerThread
    LoggingSettingsDlg
)

qt5_add_resources(resources_SRC boomerang.qrc)
add_executable(boomerang ${boomerang_SRC} ${gui_UI_H} ${resources_SRC})

target_link_libraries(boomerang
	${GC_LIBS}
	${DEBUG_LIB}
	libboomerang frontend type db boomerang_DSLs codegen util
	${CMAKE_THREAD_LIBS_INIT} boomerang_passes
)

qt5_use_modules(boomerang Core Xml Widgets)

