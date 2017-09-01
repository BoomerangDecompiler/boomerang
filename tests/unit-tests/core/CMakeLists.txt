
include(boomerang-utils)

set(TESTS
    ProjectTest
)

foreach(t ${TESTS})
	BOOMERANG_ADD_TEST(
		NAME ${t}
		SOURCES ${t}.h ${t}.cpp
		LIBRARIES
			boomerang
			${CMAKE_DL_LIBS}
			${CMAKE_THREAD_LIBS_INIT}
	)
endforeach()
