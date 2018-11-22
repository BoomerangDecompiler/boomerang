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

add_subdirectory(c/parser)

BOOMERANG_ADD_SYMBOLPROVIDER(
    NAME "C"
    SOURCES
        c/CSymbolProvider.cpp
        c/CSymbolProvider.h
    LIBRARIES
        boomerang-ansic-parser
)
