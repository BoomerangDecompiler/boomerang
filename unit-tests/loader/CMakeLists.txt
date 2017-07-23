include(boomerang-utils)

set(TESTS
	LoaderTest
	ElfLoaderTest
	SparcLoaderTest
	HpSomBinaryLoaderTest
	PalmBinaryLoaderTest
	Win32BinaryLoaderTest
)

foreach(t ${TESTS})
	BOOMERANG_ADD_TEST(
		NAME ${t}
		SOURCES ${t}.h ${t}.cpp
		LIBRARIES
			${DEBUG_LIB}
			boomerang
			boomerang-frontend
			${CMAKE_THREAD_LIBS_INIT}
	)
endforeach()

