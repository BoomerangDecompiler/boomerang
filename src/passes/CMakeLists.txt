
set(pass_SOURCES
	Pass
	RangeAnalysis
)

include_directories(${CMAKE_SOURCE_DIR}/src/db)

add_library(boomerang_passes ${pass_SOURCES})

qt5_use_modules(boomerang_passes Core)

