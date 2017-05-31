
include_directories(
	..
	../../frontend/sparc
	../../frontend/pentium
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
    FrontendTest
    FrontPentTest
    FrontSparcTest
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

