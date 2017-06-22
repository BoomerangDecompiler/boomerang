include(BOOMERANG_Macros)

set(test_LIBRARIES
	${PROTOBUF_LIBRARIES}
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

#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS codeblock.proto)

