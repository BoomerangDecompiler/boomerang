
include(boomerang-utils)

set(TESTS
    RtlTest
    CfgTest
    DfaTest
    ParserTest
    StatementTest
    ProgTest
    ProcTest
    ExpTest
)

foreach(t ${TESTS})
	BOOMERANG_ADD_TEST(
		NAME ${t}
		SOURCES ${t}.h ${t}.cpp
		LIBRARIES
			${DEBUG_LIB}
			boomerang
			${CMAKE_THREAD_LIBS_INIT}
	)
endforeach()

