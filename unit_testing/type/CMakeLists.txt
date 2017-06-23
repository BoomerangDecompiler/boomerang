include(BOOMERANG_Macros)

set(test_LIBRARIES
	${PROTOBUF_LIBRARIES}
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

#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS codeblock.proto)

