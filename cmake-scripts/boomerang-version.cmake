#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


# default Boomerang version
set(BOOMERANG_VERSION "0.4.0-alpha")

# if git is installed, update version string from git tags
set(BOOMERANG_VERSION "${PROJECT_VERSION}")
find_package(Git)
if (GIT_FOUND)
    # get current git branch
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RESULT_VARIABLE BRANCH_FAILED
        OUTPUT_VARIABLE BOOMERANG_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # get short commit hash
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RESULT_VARIABLE SHA_FAILED
        OUTPUT_VARIABLE BOOMERANG_COMMIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --dirty --tags --always
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RESULT_VARIABLE DESCRIBE_FAILED
        OUTPUT_VARIABLE BOOMERANG_VERSION_TEMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # do not overwrite default version with empty one
    if (NOT DESCRIBE_FAILED)
        set(BOOMERANG_VERSION "${BOOMERANG_VERSION_TEMP}")
    endif (NOT DESCRIBE_FAILED)
endif (GIT_FOUND)

set(PROJECT_VERSION "${BOOMERANG_VERSION}")
add_definitions(-DBOOMERANG_VERSION="${BOOMERANG_VERSION}")

message(STATUS "Configuring ${PROJECT_NAME} ${BOOMERANG_VERSION} ...")
