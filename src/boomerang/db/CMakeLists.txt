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
    db/IndirectJumpAnalyzer
    db/InsNameElem
    db/InterferenceFinder
    db/LivenessAnalyzer
    db/Module
    db/Prog
    db/Register
    db/RTL
    db/Signature
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
    db/visitor/expvisitor/BadMemofFinder
    db/visitor/expvisitor/ComplexityFinder
    db/visitor/expvisitor/ConscriptSetter
    db/visitor/expvisitor/ConstFinder
    db/visitor/expvisitor/ExpDestCounter
    db/visitor/expvisitor/ExpRegMapper
    db/visitor/expvisitor/ExpVisitor
    db/visitor/expvisitor/FlagsFinder
    db/visitor/expvisitor/MemDepthFinder
    db/visitor/expvisitor/PrimitiveTester
    db/visitor/expvisitor/UsedLocalFinder
    db/visitor/expvisitor/UsedLocsFinder

    db/visitor/CallBypasser
    db/visitor/ConstGlobalConverter
    db/visitor/DFALocalMapper
    db/visitor/ExpCastInserter
    db/visitor/ExpConstCaster
    db/visitor/ExpModifier
    db/visitor/ExpPropagator
    db/visitor/ExpSSAXformer
    db/visitor/ExpSubscripter
    db/visitor/ImplicitConverter
    db/visitor/Localiser
    db/visitor/PhiStripper
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
    db/visitor/UsedLocsVisitor
)

set(boomerang-sources "${boomerang-sources};${boomerang-db-sources}" PARENT_SCOPE)
