#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

set(TESTS
    RtlTest
    DfaTest
    ParserTest
    ProgTest
    ProcTest
    ExpTest
)

# These tests require the ELF loader
set(TESTS_WITH_ELF
    CfgTest
    StatementTest
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


if (BOOMERANG_BUILD_LOADER_Elf)
    foreach(t ${TESTS_WITH_ELF})
        BOOMERANG_ADD_TEST(
            NAME ${t}
            SOURCES ${t}.h ${t}.cpp
            LIBRARIES
                ${DEBUG_LIB}
                boomerang
                ${CMAKE_THREAD_LIBS_INIT}
        )
    endforeach()
endif (BOOMERANG_BUILD_LOADER_Elf)
