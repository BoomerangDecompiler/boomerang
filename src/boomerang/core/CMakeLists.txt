
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
