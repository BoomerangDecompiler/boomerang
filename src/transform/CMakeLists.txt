
message(WARN " TODO: Make transformation engine a part of the platform.")

set(boomerang_transform_sources
	transformer.cpp
	rdi.cpp
	generic.cpp
	transformation-parser.cpp
	transformation-scanner.cpp
	rdi.h
	generic.h
	transformation-parser.h
	transformation-scanner.h
)

add_library(boomerang_transform STATIC ${boomerang_transform_sources})
qt5_use_modules(boomerang_transform Core)

