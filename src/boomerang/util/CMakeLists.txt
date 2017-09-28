#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-util-sources
    util/Address.h
    util/Address.cpp
    util/IntervalMap.h
    util/Log.h
    util/Log.cpp
    util/Util.h
    util/Util.cpp
)

set(boomerang-sources "${boomerang-sources};${boomerang-util-sources}" PARENT_SCOPE)
