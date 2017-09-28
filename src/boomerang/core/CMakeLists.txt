#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-core-sources
    core/Boomerang.h
    core/Boomerang.cpp
    core/IBoomerang.h
    core/IProject.h
    core/Plugin.h
    core/Plugin.cpp
    core/Project.cpp
    core/Project.h
    core/Settings.h
    core/Settings.cpp
    core/Watcher.h
)

set(boomerang-sources "${boomerang-sources};${boomerang-core-sources}" PARENT_SCOPE)
