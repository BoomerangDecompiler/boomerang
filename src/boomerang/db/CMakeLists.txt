#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-db-sources
    db/BasicBlock
    db/CFG
    db/CFGCompressor
    db/DataFlow
    db/DebugInfo
    db/DefCollector
    db/Global
    db/IndirectJumpAnalyzer
    db/InsNameElem
    db/InterferenceFinder
    db/LivenessAnalyzer
    db/Module
    db/Prog
    db/ProgDecompiler
    db/Register
    db/RTL
    db/Table
    db/UseCollector

    db/binary/BinaryFile
    db/binary/BinaryImage
    db/binary/BinarySection
    db/binary/BinarySymbol
    db/binary/BinarySymbolTable

    # exp handling
    db/exp/Binary
    db/exp/Const
    db/exp/Exp
    db/exp/ExpHelp
    db/exp/FlagDef
    db/exp/Location
    db/exp/RefExp
    db/exp/Terminal
    db/exp/Ternary
    db/exp/TypedExp
    db/exp/Unary
    db/exp/Operator

    db/proc/LibProc
    db/proc/Proc
    db/proc/UserProc

    db/signature/CustomSignature
    db/signature/MIPSSignature
    db/signature/Signature
    db/signature/Parameter
    db/signature/Return
    db/signature/PentiumSignature
    db/signature/PPCSignature
    db/signature/SparcSignature
    db/signature/ST20Signature
    db/signature/Win32Signature

    db/ssl/RTLInstDict
    db/ssl/sslparser
    db/ssl/sslparser_support.cpp
    db/ssl/sslscanner

    # statement handling
    db/statements/Assign
    db/statements/Assignment
    db/statements/BoolAssign
    db/statements/BranchStatement
    db/statements/CallStatement
    db/statements/CaseStatement
    db/statements/GotoStatement
    db/statements/ImplicitAssign
    db/statements/ImpRefStatement
    db/statements/PhiAssign
    db/statements/ReturnStatement
    db/statements/Statement
    db/statements/StatementHelper
    db/statements/TypingStatement
)

set(boomerang-sources "${boomerang-sources};${boomerang-db-sources}" PARENT_SCOPE)
