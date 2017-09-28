#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-codegen-sources
    codegen/CCodeGenerator.cpp
    codegen/CCodeGenerator.h
    codegen/ICodeGenerator.h
    codegen/syntax/SyntaxNode.h
    codegen/syntax/SyntaxNode.cpp
    codegen/syntax/BlockSyntaxNode.h
    codegen/syntax/BlockSyntaxNode.cpp
    codegen/syntax/IfThenSyntaxNode.h
    codegen/syntax/IfThenSyntaxNode.cpp
    codegen/syntax/LoopSyntaxNode.h
    codegen/syntax/LoopSyntaxNode.cpp
)

set(boomerang-sources "${boomerang-sources};${boomerang-codegen-sources}" PARENT_SCOPE)
