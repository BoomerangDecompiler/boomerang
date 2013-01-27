SET(boomerang_transform_sources
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
ADD_LIBRARY(boomerang_transform STATIC ${boomerang_transform_sources})
