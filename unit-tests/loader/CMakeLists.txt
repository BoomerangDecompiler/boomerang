include(boomerang-macros)

set(test_LIBRARIES
	${GC_LIBS}
	${DEBUG_LIB}
	boomerang
	boomerang-frontend
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
	LoaderTest
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

