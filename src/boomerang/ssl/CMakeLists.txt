#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-ssl-sources
    ssl/Register
    ssl/RegDB
    ssl/RTLInstDict
    ssl/RTL
    ssl/TableEntry

    # exp handling
    ssl/exp/Binary
    ssl/exp/Const
    ssl/exp/Exp
    ssl/exp/ExpHelp
    ssl/exp/Location
    ssl/exp/RefExp
    ssl/exp/Terminal
    ssl/exp/Ternary
    ssl/exp/TypedExp
    ssl/exp/Unary
    ssl/exp/Operator

    # statement handling
    ssl/statements/Assign
    ssl/statements/Assignment
    ssl/statements/BoolAssign
    ssl/statements/BranchStatement
    ssl/statements/CallStatement
    ssl/statements/CaseStatement
    ssl/statements/GotoStatement
    ssl/statements/ImplicitAssign
    ssl/statements/PhiAssign
    ssl/statements/ReturnStatement
    ssl/statements/Statement
    ssl/statements/StatementHelper
    ssl/statements/TypingStatement

    # type handling
    ssl/type/ArrayType
    ssl/type/BooleanType
    ssl/type/CharType
    ssl/type/CompoundType
    ssl/type/FloatType
    ssl/type/FuncType
    ssl/type/IntegerType
    ssl/type/NamedType
    ssl/type/PointerType
    ssl/type/SizeType
    ssl/type/Type
    ssl/type/UnionType
    ssl/type/VoidType
)

add_subdirectory(parser)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-ssl-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-ssl-sources}" PARENT_SCOPE)
