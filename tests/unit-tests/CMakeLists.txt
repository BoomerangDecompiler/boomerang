#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


set(CMAKE_AUTOMOC ON)

# Include directories needed for all test modules
include_directories(
    "${CMAKE_SOURCE_DIR}/src/"
    "${CMAKE_BINARY_DIR}/src/"
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(boomerang-test-utils STATIC TestUtils.h TestUtils.cpp)
target_link_libraries(boomerang-test-utils Qt5::Core Qt5::Test)

add_subdirectory(boomerang)
add_subdirectory(boomerang-cli)
add_subdirectory(boomerang-plugins)
