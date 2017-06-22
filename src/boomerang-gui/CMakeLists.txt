find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

set(gui_UI_FILES
	boomerang.ui
	about.ui
	LoggingSettingsDlg.ui
)

QT5_WRAP_UI(gui_UI_H ${gui_UI_FILES})

include_directories(
	${CMAKE_SOURCE_DIR}/src/boomerang
)

SET(boomerang_SRC
    main.cpp
    mainwindow
    rtleditor
    commandlinedriver
    DecompilerThread
    LoggingSettingsDlg
)

qt5_add_resources(resources_SRC boomerang.qrc)
add_executable(boomerang-gui ${boomerang_SRC} ${gui_UI_H} ${resources_SRC})

target_link_libraries(boomerang-gui
	boomerang-frontend
	${GC_LIBS}
	${DEBUG_LIB}
	${CMAKE_THREAD_LIBS_INIT}
)

qt5_use_modules(boomerang-gui Core Xml Widgets)

