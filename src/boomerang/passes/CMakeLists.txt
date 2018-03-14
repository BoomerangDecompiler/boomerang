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

    passes/early/StatementInitPass
    passes/early/GlobalConstReplacePass
    passes/early/StatementPropagationPass
    passes/early/BBSimplifyPass

    passes/middle/CallAndPhiFixPass
    passes/middle/SPPreservationPass
    passes/middle/PreservationAnalysisPass
    passes/middle/StrengthReductionReversalPass
    passes/middle/AssignRemovalPass
    passes/middle/DuplicateArgsRemovalPass
    passes/middle/ParameterSymbolMapPass

    passes/late/CallLivenessRemovalPass
    passes/late/LocalTypeAnalysisPass
    passes/late/BranchAnalysisPass
    passes/late/FromSSAFormPass
    passes/late/FinalParameterSearchPass
    passes/late/UnusedStatementRemovalPass
    passes/late/UnusedLocalRemovalPass
    passes/late/ImplicitPlacementPass
)

set(boomerang-sources "${boomerang-sources};${boomerang-passes-sources}" PARENT_SCOPE)
