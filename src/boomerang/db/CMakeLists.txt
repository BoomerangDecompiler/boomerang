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
    db/visitor/expvisitor/TempToLocalMapper

    db/visitor/expmodifier/CallBypasser
    db/visitor/expmodifier/ConstGlobalConverter
    db/visitor/expmodifier/DFALocalMapper
    db/visitor/expmodifier/ExpCastInserter
    db/visitor/expmodifier/ExpConstCaster
    db/visitor/expmodifier/ExpModifier
    db/visitor/expmodifier/ExpPropagator
    db/visitor/expmodifier/ExpSSAXformer
    db/visitor/expmodifier/ExpSubscripter
    db/visitor/expmodifier/ImplicitConverter
    db/visitor/expmodifier/Localiser
    db/visitor/expmodifier/SimpExpModifier
    db/visitor/expmodifier/SizeStripper

    db/visitor/stmtvisitor/StmtCastInserter
    db/visitor/stmtvisitor/StmtConscriptSetter
    db/visitor/stmtvisitor/StmtVisitor

    db/visitor/stmtmodifier/PhiStripper
    db/visitor/stmtmodifier/StmtImplicitConverter
    db/visitor/stmtmodifier/StmtModifier
    db/visitor/stmtmodifier/StmtPartModifier
    db/visitor/stmtmodifier/StmtSSAXFormer
    db/visitor/stmtmodifier/StmtSubscripter

    db/visitor/stmtexpvisitor/StmtConstFinder
    db/visitor/stmtexpvisitor/StmtDestCounter
    db/visitor/stmtexpvisitor/StmtExpVisitor
    db/visitor/stmtexpvisitor/StmtRegMapper
    db/visitor/stmtexpvisitor/UsedLocsVisitor
)

set(boomerang-sources "${boomerang-sources};${boomerang-db-sources}" PARENT_SCOPE)
