set(target_INCLUDE_DIR
    ..
)

include_directories(${target_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/frontend/sparc
    ${CMAKE_SOURCE_DIR}/frontend/pentium
)

include(BOOMERANG_Macros)

set(test_LIBRARIES
	${PROTOBUF_LIBRARIES}
	${GC_LIBS}
	${DEBUG_LIB}
	boomerang_DSLs
	type db codegen boomerang_passes type util boom_base frontend db codegen
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
    CTest
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS codeblock.proto)

