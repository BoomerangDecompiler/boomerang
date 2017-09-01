include(boomerang-utils)

set(TESTS
	ElfBinaryLoaderTest
	SparcBinaryLoaderTest
	HpSomBinaryLoaderTest
	PalmBinaryLoaderTest
	Win32BinaryLoaderTest
	MicroX86DisTest
)


foreach(t ${TESTS})
	BOOMERANG_ADD_TEST(
		NAME ${t}
		SOURCES ${t}.h ${t}.cpp
		LIBRARIES
			${DEBUG_LIB}
			boomerang
			${CMAKE_THREAD_LIBS_INIT}
	)
endforeach()

