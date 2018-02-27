#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-passes-sources
    passes/Pass
    passes/PassGroup
    passes/PassManager

    passes/dataflow/DominatorPass
    passes/dataflow/PhiPlacementPass
    passes/dataflow/BlockVarRenamePass

    passes/call/CallDefineUpdatePass
    passes/call/CallArgumentUpdatePass
)

set(boomerang-sources "${boomerang-sources};${boomerang-passes-sources}" PARENT_SCOPE)
