include(boomerang-utils)

set(test_LIBRARIES
	${GC_LIBS}
	${DEBUG_LIB}
	boomerang
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
    TypeTest
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

