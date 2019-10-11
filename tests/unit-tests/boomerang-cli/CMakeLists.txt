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

BOOMERANG_ADD_TEST(
    NAME CommandLineDriverTest
    SOURCES
        ${CMAKE_SOURCE_DIR}/src/boomerang-cli/CommandlineDriver.cpp
        ${CMAKE_SOURCE_DIR}/src/boomerang-cli/CommandlineDriver.h
        ${CMAKE_SOURCE_DIR}/src/boomerang-cli/Console.cpp
        ${CMAKE_SOURCE_DIR}/src/boomerang-cli/Console.h

        ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineDriverTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineDriverTest.h
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)
