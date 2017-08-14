
include(boomerang-utils)

set(TESTS
    CTest
)

foreach(t ${TESTS})
	BOOMERANG_ADD_TEST(
		NAME ${t}
		SOURCES ${t}.h ${t}.cpp
		LIBRARIES
			boomerang
			boomerang-frontend
			${CMAKE_DL_LIBS}
			${CMAKE_THREAD_LIBS_INIT}
	)
endforeach()

