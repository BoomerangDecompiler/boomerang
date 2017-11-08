#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-type-sources
    type/type/ArrayType
    type/type/BooleanType
    type/type/CharType
    type/type/CompoundType
    type/type/FloatType
    type/type/FuncType
    type/type/IntegerType
    type/type/NamedType
    type/type/PointerType
    type/type/SizeType
    type/type/Type
    type/type/UnionType
    type/type/VoidType

    type/DataIntervalMap
    type/TypeRecovery

    # DFA type recovery
    type/dfa/DFATypeRecovery
    type/dfa/DFATypeAnalyzer
)

set(boomerang-sources "${boomerang-sources};${boomerang-type-sources}" PARENT_SCOPE)
