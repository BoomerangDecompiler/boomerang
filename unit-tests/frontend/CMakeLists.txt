
include(boomerang-macros)

set(TESTS
    FrontendTest
    FrontPentTest
    FrontSparcTest
)

foreach(t ${TESTS})
	BOOMERANG_ADD_TEST(
		NAME ${t}
		SOURCES ${t}.h ${t}.cpp
		LIBRARIES
			${DEBUG_LIB}
			boomerang-frontend
			${CMAKE_DL_LIBS}
			${CMAKE_THREAD_LIBS_INIT}
	)
endforeach()

