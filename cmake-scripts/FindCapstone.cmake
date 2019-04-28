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
#   Capstone_VERSION         - Capstone library version
#   Capstone_INCLUDE_DIRS    - Include directories needed for Capstone
#   Capstone_LIBRARIES       - Libraries to link to when using Capstone
#   Capstone_DLL             - Path to Capstone DLL, if applicable
#   Capstone_PDB             - Path to Capstone PDB, if applicable
#   Capstone_CSTOOL          - Path to `cstool` executable, if present
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

set(Capstone_NAMES capstone capstone_dll)
find_library(Capstone_LIBRARY
    NAMES ${Capstone_NAMES}
    PATHS /usr/local/lib /usr/lib/ $ENV{MINGDIR}/lib
)

if (WIN32)
    foreach (DLLNAME ${Capstone_NAMES})
        find_file(Capstone_DLL
            NAME ${DLLNAME}.dll
            PATHS /usr/local/bin /usr/bin/ $ENV{MINGDIR}/bin
        )

        find_file(Capstone_PDB
            NAME ${DLLNAME}.pdb
            PATHS /usr/local/bin /usr/bin/ $ENV{MINGDIR}/bin
        )
    endforeach ()
endif (WIN32)


# find cstool and extract version information, if available
find_program(Capstone_CSTOOL
    NAMES cstool.exe cstool
    PATHS /usr/local/bin /usr/bin/ $ENV{MINGDIR}/bin
)


if (Capstone_CSTOOL)
    execute_process(COMMAND ${Capstone_CSTOOL} -v
        OUTPUT_VARIABLE CSTOOL_OUTPUT
    )

    if (CSTOOL_OUTPUT MATCHES "v([0-9]+).([0-9]+).([0-9]+)")
        set(Capstone_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
        set(Capstone_VERSION_MAJOR "${CMAKE_MATCH_1}")
        set(Capstone_VERSION_MINOR "${CMAKE_MATCH_2}")
        set(Capstone_VERSION_PATCH "${CMAKE_MATCH_3}")
    endif ()
endif (Capstone_CSTOOL)


if (WIN32)
    # Allow dll to be built without debug symbol support
    find_package_handle_standard_args(Capstone
        FOUND_VAR Capstone_FOUND
        VERSION_VAR Capstone_VERSION
        REQUIRED_VARS Capstone_LIBRARY Capstone_INCLUDE_DIR Capstone_DLL
    )
else (WIN32)
    find_package_handle_standard_args(Capstone
        FOUND_VAR Capstone_FOUND
        VERSION_VAR Capstone_VERSION
        REQUIRED_VARS Capstone_LIBRARY Capstone_INCLUDE_DIR
    )
endif (WIN32)


set(Capstone_INCLUDE_DIRS "${Capstone_INCLUDE_DIR}")
set(Capstone_LIBRARIES "${Capstone_LIBRARY}")

if (Capstone_FOUND OR NOT Capstone_FIND_REQUIRED)
    # Only show variables when Capstone is required and not found
    mark_as_advanced(
        Capstone_INCLUDE_DIRS
        Capstone_LIBRARIES
        Capstone_CONFIG
        Capstone_INCLUDE_DIR
        Capstone_LIBRARY
        Capstone_CSTOOL
    )

    if (WIN32)
        mark_as_advanced(Capstone_DLL Capstone_PDB)
    endif (WIN32)
endif (Capstone_FOUND OR NOT Capstone_FIND_REQUIRED)

if (Capstone_FOUND)
    if (NOT TARGET Capstone::Capstone)
        add_library(Capstone::Capstone UNKNOWN IMPORTED)
    endif (NOT TARGET Capstone::Capstone)

    set_target_properties(Capstone::Capstone PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Capstone_INCLUDE_DIRS}")
    set_target_properties(Capstone::Capstone PROPERTIES IMPORTED_LOCATION "${Capstone_LIBRARIES}")
endif (Capstone_FOUND)
