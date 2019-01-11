#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

set(TESTS
    exp/ExpTest
    parser/ParserTest
    type/ArrayTypeTest
    type/BooleanTypeTest
    type/CharTypeTest
    type/CompoundTypeTest
    type/FloatTypeTest
    type/FuncTypeTest
    type/NamedTypeTest
    type/IntegerTypeTest
    type/PointerTypeTest
    type/MeetTest
    RegDBTest
    RTLTest
    type/UnionTypeTest
)


# These tests require the ELF loader
set(TESTS_WITH_ELF
    statements/StatementTest
)

set(TESTS_WITH_WIN32
    type/TypeTest
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


if (BOOMERANG_BUILD_LOADER_Win32)
    foreach(t ${TESTS_WITH_WIN32})
        BOOMERANG_ADD_TEST(
            NAME ${t}
            SOURCES ${t}.h ${t}.cpp
            LIBRARIES
                ${DEBUG_LIB}
                boomerang
                ${CMAKE_THREAD_LIBS_INIT}
        )
    endforeach()
endif (BOOMERANG_BUILD_LOADER_Win32)
