
SET(SRC
	util.h
	util.cpp
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(util STATIC ${SRC})
qt5_use_modules(util Core)

