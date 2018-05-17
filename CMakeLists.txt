#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

cmake_minimum_required(VERSION 3.8)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    # There is also the RULE_LAUNCH_LINK property, however ccache only passes the input to the linker. So don't enable ccache on link time.
endif (CCACHE_PROGRAM)

project(Boomerang LANGUAGES C CXX)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

set(CMAKE_WARN_DEPRECATED ON)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake-scripts")

# CMake does not fill in CMAKE_BUILD_TYPE automatically.
if (NOT MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type. Valid values are Debug Release MinSizeRel RelWithDebInfo" FORCE)
    endif ()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif (NOT MSVC)


include(boomerang-version)      # get project version
include(boomerang-dependencies) # library dependencies
include(boomerang-configure)    # boomerang configuration (options)
include(boomerang-flags)        # find required compiler flags
include(boomerang-output)       # compiler output directories

add_subdirectory(docs)
add_subdirectory(src)

include(boomerang-tests)
include(boomerang-install)
