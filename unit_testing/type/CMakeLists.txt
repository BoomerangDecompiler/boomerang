include(BOOMERANG_Macros)

set(target_INCLUDE_DIR
    ..
)

include_directories(${target_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/frontend/sparc
    ${CMAKE_SOURCE_DIR}/src/frontend/pentium
)

set(test_LIBRARIES
	${PROTOBUF_LIBRARIES}
	${GC_LIBS}
	${DEBUG_LIB}
	boom_base frontend db type boomerang_DSLs codegen util
	boom_base frontend db codegen boomerang_passes
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
    TypeTest
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS codeblock.proto)

