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

BOOMERANG_ADD_TYPERECOVERY(
    NAME "DFA"
    SOURCES
        dfa/DFATypeAnalyzer.cpp
        dfa/DFATypeAnalyzer.h
        dfa/DFATypeRecovery.cpp
        dfa/DFATypeRecovery.h
        dfa/TypeRecovery.cpp
        dfa/TypeRecovery.h
)

