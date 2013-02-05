INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/platform/include/db)
MESSAGE(WARN " TODO: Make transformation engine a part of the platform.")
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
