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

if (BOOMERANG_BUILD_CLI)
    add_subdirectory(boomerang-cli)
endif (BOOMERANG_BUILD_CLI)

if (BOOMERANG_BUILD_GUI)
    add_subdirectory(boomerang-gui)
endif (BOOMERANG_BUILD_GUI)

# plugins
add_subdirectory(boomerang-plugins/codegen)
add_subdirectory(boomerang-plugins/decoder)
add_subdirectory(boomerang-plugins/frontend)
add_subdirectory(boomerang-plugins/loader)
add_subdirectory(boomerang-plugins/symbol)
add_subdirectory(boomerang-plugins/type)
