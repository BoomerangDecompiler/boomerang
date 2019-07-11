#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-core-sources
    core/BoomerangAPI
    core/Project
    core/Settings
    core/Watcher
    core/plugin/Plugin
    core/plugin/PluginHandle
    core/plugin/PluginManager
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-core-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-core-sources}" PARENT_SCOPE)
