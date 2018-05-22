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
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/")

if (WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif ()

file(MAKE_DIRECTORY ${BOOMERANG_OUTPUT_DIR}/bin)
file(MAKE_DIRECTORY ${BOOMERANG_OUTPUT_DIR}/share)

function(BOOMERANG_LINK_DIRECTORY LINK_SOURCE LINK_DEST)
    # link output data directory to ${CMAKE_SOURCE_DIR}/data"
    if (WIN32)
        string(REPLACE "/" "\\\\" LNK_LOC    "${LINK_SOURCE}")
        string(REPLACE "/" "\\\\" LNK_TARGET "${LINK_DEST}")

        # Do not invoke mklink directly. If invoked directly, mklink will fail if the link already exists.
        # But we only want to know if mklink fails because of some other reason.
        set(LNK_COMMAND "if not exist ${LNK_LOC} (mklink /J ${LNK_LOC} ${LNK_TARGET})")
        execute_process(COMMAND "cmd" /C "${LNK_COMMAND}"
            WORKING_DIRECTORY "${BOOMERANG_OUTPUT_DIR}"
            OUTPUT_QUIET
            ERROR_VARIABLE LNK_ERROR
            ERROR_STRIP_TRAILING_WHITESPACE
        )

        if (LNK_ERROR)
            message(WARNING "Could not link to data directory:\n"
                "Command '${LNK_COMMAND}' failed with\n"
                "error message '${LNK_ERROR}'")
        endif (LNK_ERROR)
    else () # Linux
        execute_process(COMMAND ln -sfn "${LINK_DEST}" "${LINK_SOURCE}")
    endif ()
endfunction()

BOOMERANG_LINK_DIRECTORY("${BOOMERANG_OUTPUT_DIR}/share/boomerang" "${CMAKE_SOURCE_DIR}/data")
BOOMERANG_LINK_DIRECTORY("${CMAKE_BINARY_DIR}/tests/regression-tests/desired-outputs" "${CMAKE_SOURCE_DIR}/tests/regression-tests/desired-outputs")

# delete all files in the 'out/' directory on make clean
set(EXTRA_CLEAN_FILES "${BOOMERANG_OUTPUT_DIR}")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${EXTRA_CLEAN_FILES}")
