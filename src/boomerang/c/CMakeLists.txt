#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-c-sources
    c/ansi-c-parser.cpp
    c/ansi-c-scanner.cpp
    c/ansi-c-parser.h
    c/ansi-c-scanner.h
)

set(boomerang-sources "${boomerang-sources};${boomerang-c-sources}" PARENT_SCOPE)
