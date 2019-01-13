#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

set(test_LIBRARIES
    ${GC_LIBS}
    ${DEBUG_LIB}
    boomerang
    ${CMAKE_THREAD_LIBS_INIT}
)


set(TESTS
    expmodifier/ExpAddrSimplifierTest
    expmodifier/ExpArithSimplifierTest
    expmodifier/ExpSimplifierTest
    stmtexpvisitor/StmtConstFinderTest
    stmtmodifier/StmtSubscripterTest
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
