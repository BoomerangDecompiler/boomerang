
include_directories(
    ${CMAKE_SOURCE_DIR}/src/frontend/sparc
    ${CMAKE_SOURCE_DIR}/src/frontend/pentium
)

include(BOOMERANG_Macros)

set(test_LIBRARIES
	${PROTOBUF_LIBRARIES}
	${GC_LIBS}
	${DEBUG_LIB}
	boomerang_DSLs
	type db boomerang_passes util frontend
	libboomerang
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
    CTest
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

