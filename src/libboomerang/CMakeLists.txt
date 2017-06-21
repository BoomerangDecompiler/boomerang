
set(libboomerang_SRC
	core/Boomerang.h
	core/Boomerang.cpp
	core/Log.h
	core/Log.cpp
	core/BinaryFileFactory.cpp
	core/Plugin.h
)

add_library(libboomerang ${libboomerang_SRC})
target_link_libraries(libboomerang ${CMAKE_DL_LIBS})

set_target_properties(libboomerang PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

