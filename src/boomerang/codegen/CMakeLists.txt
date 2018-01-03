#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-codegen-sources
    codegen/CCodeGenerator
    codegen/ControlFlowAnalyzer
    codegen/ICodeGenerator.h
    codegen/syntax/SyntaxNode
    codegen/syntax/BlockSyntaxNode
    codegen/syntax/IfThenSyntaxNode
    codegen/syntax/LoopSyntaxNode
)

set(boomerang-sources "${boomerang-sources};${boomerang-codegen-sources}" PARENT_SCOPE)
