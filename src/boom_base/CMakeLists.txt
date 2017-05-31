
set(boom_base_SRC
	boomerang.h
	boomerang.cpp
	log.h
	log.cpp
	BinaryFile.h
	BinaryFileFactory.cpp
	BinaryFileStub.h
)

add_library(boom_base ${boom_base_SRC})

qt5_use_modules(boom_base Core Xml)
set_target_properties(boom_base PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

