#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} # for generated source files
)


add_subdirectory(boomerang)
add_subdirectory(boomerang-cli)
add_subdirectory(boomerang-gui)

# plugins
add_subdirectory(boomerang-loaders)

