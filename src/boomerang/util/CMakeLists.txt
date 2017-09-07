
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
