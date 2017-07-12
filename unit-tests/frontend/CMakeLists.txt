
set(test_LIBRARIES
	${PROTOBUF_LIBRARIES}
	${GC_LIBS}
	${DEBUG_LIB}
	boomerang-frontend
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
    FrontendTest
    FrontPentTest
    FrontSparcTest
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

