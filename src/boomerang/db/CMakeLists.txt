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
    db/DefCollector
    db/Global
    db/IBinarySection.h
    db/IBinarySymbols.h
    db/IndirectJumpAnalyzer
    db/InsNameElem
    db/InterferenceFinder
    db/LivenessAnalyzer
    db/Module
    db/Prog
    db/Register
    db/RTL
    db/SectionInfo
    db/Signature
    db/SymTab
    db/Table
    db/UseCollector

    db/binary/BinaryImage

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

    # visitors
    db/visitor/BadMemofFinder
    db/visitor/CallBypasser
    db/visitor/ComplexityFinder
    db/visitor/ConscriptSetter
    db/visitor/ConstFinder
    db/visitor/ConstGlobalConverter
    db/visitor/DFALocalMapper
    db/visitor/ExpCastInserter
    db/visitor/ExpConstCaster
    db/visitor/ExpDestCounter
    db/visitor/ExpModifier
    db/visitor/ExpPropagator
    db/visitor/ExpRegMapper
    db/visitor/ExpSSAXformer
    db/visitor/ExpSubscripter
    db/visitor/ExpVisitor
    db/visitor/FlagsFinder
    db/visitor/ImplicitConverter
    db/visitor/Localiser
    db/visitor/MemDepthFinder
    db/visitor/PhiStripper
    db/visitor/PrimitiveTester
    db/visitor/SimpExpModifier
    db/visitor/SizeStripper
    db/visitor/StmtCastInserter
    db/visitor/StmtConscriptSetter
    db/visitor/StmtConstFinder
    db/visitor/StmtDestCounter
    db/visitor/StmtExpVisitor
    db/visitor/StmtImplicitConverter
    db/visitor/StmtModifier
    db/visitor/StmtPartModifier
    db/visitor/StmtRegMapper
    db/visitor/StmtSSAXFormer
    db/visitor/StmtSubscripter
    db/visitor/StmtVisitor
    db/visitor/TempToLocalMapper
    db/visitor/UsedLocalFinder
    db/visitor/UsedLocsFinder
    db/visitor/UsedLocsVisitor
)

set(boomerang-sources "${boomerang-sources};${boomerang-db-sources}" PARENT_SCOPE)
