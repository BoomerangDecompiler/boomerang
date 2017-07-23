
include(boomerang-utils)

set(test_LIBRARIES
	${DEBUG_LIB}
	boomerang
	boomerang-frontend
	${CMAKE_THREAD_LIBS_INIT}
)

set(TESTS
    RtlTest
    CfgTest
    DfaTest
    ParserTest
    StatementTest
    ProgTest
    ExpTest
)

foreach(t ${TESTS})
	BOOMERANG_ADD_TEST(
		NAME ${t}
		SOURCES ${t}.h ${t}.cpp
		LIBRARIES
			${DEBUG_LIB}
			boomerang
			boomerang-frontend
			${CMAKE_THREAD_LIBS_INIT}
	)
endforeach()

