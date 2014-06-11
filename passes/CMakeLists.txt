set(pass_SOURCES
Pass
RangeAnalysis
)

add_library(boomerang_passes ${pass_SOURCES})
qt5_use_modules(boomerang_passes Core)
