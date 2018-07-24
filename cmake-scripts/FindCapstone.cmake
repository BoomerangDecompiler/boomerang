#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#
# - Find Capstone disassembler library.
#
# This module defines the following variables:
#   Capstone_FOUND           - true if Capstone was found
#   Capstone_INCLUDE_DIRS    - Include directories needed for Capstone
#   Capstone_LIBRARIES       - Libraries to link to when using Capstone
#
# Additionally, this module defines the IMPORTED target Capstone::Capstone,
# if Capstone has been found.
#

include(FindPackageHandleStandardArgs)

if (Capstone_INCLUDE_DIRS AND Capstone_LIBRARIES)
    set(Capstone_FIND_QUIETLY TRUE)
endif (Capstone_INCLUDE_DIRS AND Capstone_LIBRARIES)

find_path(Capstone_INCLUDE_DIR capstone/capstone.h
    /usr/local/include
    /usr/include
    $ENV{MINGDIR}/include
)

set(Capstone_NAMES capstone)
find_library(Capstone_LIBRARY
    NAMES ${Capstone_NAMES}
    PATHS /usr/local/lib /usr/lib/ $ENV{MINGDIR}/lib
)


find_package_handle_standard_args(Capstone
    FOUND_VAR Capstone_FOUND
    REQUIRED_VARS Capstone_LIBRARY Capstone_INCLUDE_DIR
)


set(Capstone_INCLUDE_DIRS "${Capstone_INCLUDE_DIR}")
set(Capstone_LIBRARIES "${Capstone_LIBRARY}")

if (Capstone_FOUND OR NOT Capstone_FIND_REQUIRED)
    # Only show variables when Capstone is required and not found
    mark_as_advanced(Capstone_INCLUDE_DIRS Capstone_LIBRARIES Capstone_CONFIG Capstone_INCLUDE_DIR Capstone_LIBRARY)
endif (Capstone_FOUND OR NOT Capstone_FIND_REQUIRED)

if (Capstone_FOUND)
    if (NOT TARGET Capstone::Capstone)
        add_library(Capstone::Capstone UNKNOWN IMPORTED)
    endif (NOT TARGET Capstone::Capstone)

    set_target_properties(Capstone::Capstone PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Capstone_INCLUDE_DIRS}")
    set_target_properties(Capstone::Capstone PROPERTIES IMPORTED_LOCATION "${Capstone_LIBRARIES}")
endif (Capstone_FOUND)

