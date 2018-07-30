#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-c-sources
    c/parser/AnsiCParser
    c/parser/AnsiCScanner
    c/CSymbolProvider
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-c-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-c-sources}" PARENT_SCOPE)
