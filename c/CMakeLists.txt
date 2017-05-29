
SET(boomerang_DSLs_sources
	ansi-c-parser.cpp
	ansi-c-scanner.cpp
	ansi-c-parser.h
	ansi-c-scanner.h
)

add_library(boomerang_DSLs STATIC ${boomerang_DSLs_sources})
qt5_use_modules(boomerang_DSLs Core)

if (BUILD_TESTING)
	add_subdirectory(unit_testing)
endif ()

