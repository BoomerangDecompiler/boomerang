
include(boomerang-macros)

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
)

foreach(t ${TESTS})
	ADD_QTEST(${t})
endforeach()

