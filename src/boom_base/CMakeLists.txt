
set(boom_base_SRC
	boomerang.cpp
	log.cpp
	"${CMAKE_SOURCE_DIR}/loader/BinaryFileFactory.cpp"
)

add_library(boom_base ${boom_base_SRC})

qt5_use_modules(boom_base Core Xml)
set_target_properties(boom_base PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

