
set(boomerang_c_codegen_sources
    chllcode.cpp
    syntax.cpp
	chllcode.h
)

add_library(codegen STATIC ${boomerang_c_codegen_sources})
qt5_use_modules(codegen Core)

