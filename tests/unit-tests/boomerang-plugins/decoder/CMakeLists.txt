#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

include(boomerang-utils)

BOOMERANG_ADD_TEST(
    NAME CapstonePPCDecoderTest
    SOURCES
        ppc/CapstonePPCDecoderTest.h
        ppc/CapstonePPCDecoderTest.cpp
    LIBRARIES
        boomerang-CapstonePPCDecoder
    DEPENDENCIES
        boomerang-CapstonePPCDecoder
)
