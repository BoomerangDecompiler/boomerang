#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

cmake_minimum_required(VERSION 3.1)
project(Boomerang LANGUAGES C CXX)

set(CMAKE_WARN_DEPRECATED ON)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake-scripts")

# CMake does not fill in CMAKE_BUILD_TYPE automatically.
if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type. Valid values are Debug Release MinSizeRel RelWithDebInfo")
endif ()


include(boomerang-version)      # get project version
include(boomerang-dependencies) # library dependencies
include(boomerang-configure)    # boomerang configuration (options)
include(boomerang-flags)        # find required compiler flags
include(boomerang-output)       # compiler output directories

add_subdirectory(docs)
add_subdirectory(src)

include(boomerang-tests)
include(boomerang-install)
