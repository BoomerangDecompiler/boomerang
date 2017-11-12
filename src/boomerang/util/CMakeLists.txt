#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-util-sources
    util/Address
    util/AssignSet
    util/ConnectionGraph
    util/Interval.h
    util/IntervalMap.h
    util/IntervalSet.h
    util/LocationSet
    util/Log
    util/StatementList
    util/StatementSet
    util/StatementVec
    util/Types.h
    util/Util
)

set(boomerang-sources "${boomerang-sources};${boomerang-util-sources}" PARENT_SCOPE)
