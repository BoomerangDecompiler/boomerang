
set(pass_SOURCES
	Pass
	RangeAnalysis
)

add_library(boomerang_passes ${pass_SOURCES})

target_link_libraries(boomerang_passes db)

qt5_use_modules(boomerang_passes Core)

