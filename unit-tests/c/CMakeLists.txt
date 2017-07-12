
include(boomerang-macros)

set(test_LIBRARIES
	${PROTOBUF_LIBRARIES}
	${GC_LIBS}
	${DEBUG_LIB}
	boomerang
	boomerang-frontend
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
    CTest
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

