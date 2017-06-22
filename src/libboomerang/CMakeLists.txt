
set(libboomerang_SRC
	codegen/CCodeGenerator.cpp
	codegen/CCodeGenerator.h
	codegen/ICodeGenerator.h
	codegen/SyntaxNode.h
	codegen/SyntaxNode.cpp
	
	core/Boomerang.h
	core/Boomerang.cpp
	core/BinaryFileFactory.cpp
	core/Plugin.h
	
	util/Log.h
	util/Log.cpp
	util/Util.h
	util/Util.cpp
)

add_library(libboomerang ${libboomerang_SRC})
target_link_libraries(libboomerang frontend ${CMAKE_DL_LIBS})

set_target_properties(libboomerang PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

