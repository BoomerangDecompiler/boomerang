#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

include(boomerang-utils)

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
)

BOOMERANG_ADD_FRONTEND(
    NAME "X86"
    SOURCES
        x86/PentiumFrontEnd.cpp
        x86/PentiumFrontEnd.h
        x86/StringInstructionProcessor.cpp
        x86/StringInstructionProcessor.h
)

BOOMERANG_ADD_FRONTEND(
    NAME "SPARC"
    SOURCES
        sparc/SPARCFrontEnd.cpp
        sparc/SPARCFrontEnd.h
)

BOOMERANG_ADD_FRONTEND(
    NAME "PPC"
    SOURCES
        ppc/PPCFrontEnd.cpp
        ppc/PPCFrontEnd.h
)

BOOMERANG_ADD_FRONTEND(
    NAME "ST20"
    SOURCES
        st20/ST20FrontEnd.cpp
        st20/ST20FrontEnd.h
)
