#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


# Base directory for all output files
set(BOOMERANG_OUTPUT_DIR "${PROJECT_BINARY_DIR}/out")

set(CMAKE_SHARED_MODULE_PREFIX "") # prevent windows/mingw modules having lib* prefix
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/")

if (WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif ()

file(MAKE_DIRECTORY ${BOOMERANG_OUTPUT_DIR}/bin)

# copy function signatures to output directory
file(COPY "${CMAKE_SOURCE_DIR}/data/signatures/" DESTINATION "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/signatures/")
file(COPY "${CMAKE_SOURCE_DIR}/data/ssl/"        DESTINATION "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/ssl/")


# delete all files in the 'out/' directory on make clean
set(EXTRA_CLEAN_FILES "${BOOMERANG_OUTPUT_DIR}")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${EXTRA_CLEAN_FILES}")
