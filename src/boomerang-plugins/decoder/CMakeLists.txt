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

BOOMERANG_ADD_DECODER(
    NAME "CapstoneX86"
    SOURCES
        csx86/CapstoneX86Decoder.cpp
        csx86/CapstoneX86Decoder.h
        csx86/CapstoneDecoder.cpp
        csx86/CapstoneDecoder.h
)

BOOMERANG_ADD_DECODER(
    NAME "PPC"
    SOURCES
        ppc/PPCDecoder.cpp
        ppc/PPCDecoder.h
)

BOOMERANG_ADD_DECODER(
    NAME "SPARC"
    SOURCES
        sparc/SPARCDecoder.cpp
        sparc/SPARCDecoder.h
)

BOOMERANG_ADD_DECODER(
    NAME "ST20"
    SOURCES
        st20/ST20Decoder.cpp
        st20/ST20Decoder.h
)
