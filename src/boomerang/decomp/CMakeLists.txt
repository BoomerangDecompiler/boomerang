#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-decomp-sources
    decomp/CFGCompressor
    decomp/IndirectJumpAnalyzer
    decomp/InterferenceFinder
    decomp/LivenessAnalyzer
    decomp/ProcDecompiler
    decomp/ProgDecompiler
    decomp/UnusedReturnRemover
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-decomp-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-decomp-sources}" PARENT_SCOPE)
