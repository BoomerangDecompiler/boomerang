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
    LIBRARIES
        Capstone::Capstone
)

if (MSVC AND BOOMERANG_BUILD_DECODER_CapstoneX86)
    add_custom_command(TARGET boomerang-CapstoneX86Decoder POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        ARGS
            -E copy_if_different
            "${Capstone_DLL}"
            "${BOOMERANG_OUTPUT_DIR}/bin/"
    )

    install(FILES "${Capstone_DLL}" DESTINATION "bin/")
endif ()


BOOMERANG_ADD_DECODER(
    NAME "CapstonePPC"
    SOURCES
        csx86/CapstoneDecoder.cpp
        csx86/CapstoneDecoder.h
        ppc/CapstonePPCDecoder.cpp
        ppc/CapstonePPCDecoder.h
    LIBRARIES
        Capstone::Capstone
)

if (MSVC AND BOOMERANG_BUILD_DECODER_CapstonePPC)
    add_custom_command(TARGET boomerang-CapstonePPCDecoder POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        ARGS
            -E copy_if_different
            "${Capstone_DLL}"
            "${BOOMERANG_OUTPUT_DIR}/bin/"
    )

    install(FILES "${Capstone_DLL}" DESTINATION "bin/")
endif ()


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
