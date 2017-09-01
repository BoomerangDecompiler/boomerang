
list(APPEND boomerang-codegen-sources
    codegen/CCodeGenerator.cpp
    codegen/CCodeGenerator.h
    codegen/ICodeGenerator.h
    codegen/SyntaxNode.h
    codegen/SyntaxNode.cpp
)

set(boomerang-sources "${boomerang-sources};${boomerang-codegen-sources}" PARENT_SCOPE)
