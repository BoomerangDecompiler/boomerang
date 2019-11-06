#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

BOOMERANG_ADD_TEST(
    NAME ProjectTest
    SOURCES ProjectTest.h ProjectTest.cpp
    LIBRARIES boomerang ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT}
    DEPENDENCIES
        boomerang-ElfLoader
        boomerang-X86FrontEnd
)
