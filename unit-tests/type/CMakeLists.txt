include(boomerang-macros)

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
	ADD_QTEST(${t})
endforeach()

