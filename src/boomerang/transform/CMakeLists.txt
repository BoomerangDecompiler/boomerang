#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-transform-sources
    transform/ExpTransformer.cpp
    transform/ExpTransformer.h
    transform/GenericExpTransformer.cpp
    transform/GenericExpTransformer.h
    transform/RDIExpTransformer.cpp
    transform/RDIExpTransformer.h
    transform/transformation-parser.cpp
    transform/transformation-parser.h
    transform/transformation-scanner.cpp
    transform/transformation-scanner.h
)

set(boomerang-sources "${boomerang-sources};${boomerang-transform-sources}" PARENT_SCOPE)
