
include(boomerang-macros)

BOOMERANG_ADD_TEST(
	NAME g_tests
	SOURCES
		basic_tests
		testloader
	LIBRARIES
		${DEBUG_LIB}
		${CMAKE_DL_LIBS}
		${CMAKE_THREAD_LIBS_INIT}
)

