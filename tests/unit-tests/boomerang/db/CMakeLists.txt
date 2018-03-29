#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

set(TESTS
    BasicBlockTest
    binary/BinaryImageTest
    binary/BinarySectionTest
    binary/BinarySymbolTableTest
    binary/BinarySymbolTest
    CFGTest
    DfaTest
    ExpTest
    ParserTest
    ProgTest
    ProcTest
    RtlTest
)

# These tests require the ELF loader
set(TESTS_WITH_ELF
    DataFlowTest
    StatementTest
)

foreach(t ${TESTS})
    string(REGEX REPLACE ".*/" "" TEST_NAME ${t})
	BOOMERANG_ADD_TEST(
		NAME ${TEST_NAME}
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
