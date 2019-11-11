#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)


BOOMERANG_ADD_TEST(
    NAME X86FrontEndTest
    SOURCES X86FrontEndTest.h X86FrontEndTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
        boomerang-X86FrontEnd
    DEPENDENCIES
        boomerang-ElfLoader
        boomerang-X86FrontEnd
)

BOOMERANG_ADD_TEST(
    NAME SPARCFrontEndTest
    SOURCES SPARCFrontEndTest.h SPARCFrontEndTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
        boomerang-SPARCFrontEnd
    DEPENDENCIES
        boomerang-ElfLoader
        boomerang-SPARCFrontEnd
)
