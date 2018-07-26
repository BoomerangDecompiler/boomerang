#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-type-sources
    type/DataIntervalMap
    type/TypeRecovery

    # DFA type recovery
    type/dfa/DFATypeRecovery
    type/dfa/DFATypeAnalyzer
)

set(boomerang-sources "${boomerang-sources};${boomerang-type-sources}" PARENT_SCOPE)
