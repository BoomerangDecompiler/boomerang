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
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-codegen-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-codegen-sources}" PARENT_SCOPE)
