cmake_minimum_required(VERSION 3.1)
project(Boomerang VERSION 0.4.0 LANGUAGES C CXX)
set(CMAKE_WARN_DEPRECATED ON)

###############################################################################
# Settings related to build system, and target directories
###############################################################################

# if git is installed, update version string from git tags
find_package(Git)
if (GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --dirty --tags --always
        RESULT_VARIABLE DESCRIBE_FAILED
        OUTPUT_VARIABLE VERSION_TEMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (NOT DESCRIBE_FAILED)
        set(PROJECT_VERSION   "${VERSION_TEMP}")
        set(BOOMERANG_VERSION "${VERSION_TEMP}")
    endif (NOT DESCRIBE_FAILED)
endif (GIT_FOUND)


message(STATUS "Configuring " ${PROJECT_NAME} " " ${BOOMERANG_VERSION})
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

# Set a single output directory for generated binaries.
# This is not ideal, but necessary for tests.
set(BOOMERANG_OUTPUT_DIR "${PROJECT_BINARY_DIR}/out")

set(CMAKE_SHARED_MODULE_PREFIX "") # prevent windows/mingw modules having lib* prefix
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib")

if (WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif ()


###############################################################################
# Options section
###############################################################################
option(BOOMERANG_BUILD_TESTS "Build the testing tree." ON)

# CMake does not fill in CMAKE_BUILD_TYPE automatically.
if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type. Valid values are Debug Release MinSizeRel RelWithDebInfo")
endif ()

# help the cmake find 3rd_party compiled/installed packages
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/3rd_party")


###############################################################################
# Defines/configuration section
###############################################################################

add_definitions(-DBOOMERANG_VERSION="${BOOMERANG_VERSION}")
include(boomerang-configure)

# Check for big/little endian
if (WORDS_BIGENDIAN)
    add_definitions(-DBOOMERANG_BIG_ENDIAN=1)
else ()
    add_definitions(-DBOOMERANG_BIG_ENDIAN=0)
endif ()

# Check 32/64 bit system
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DBOOMERANG_BIT_COUNT=64)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DBOOMERANG_BIT_COUNT=32)
else ()
    message(FATAL_ERROR "Unknown platform with sizeof(void*) == " ${CMAKE_SIZEOF_VOID_P})
endif ()

# Define this to 1 if you want to use Dominance Numbers for analysis
add_definitions(-DUSE_DOMINANCE_NUMS=0)
add_definitions(-DBCCTR_LONG=0)
add_definitions(-DYYMAXDEPTH=10000)
add_definitions(-DSYMS_IN_BACK_END=0)
add_definitions(-DDEBUG_SPLIT_FOR_BRANCH=0)
add_definitions(-DNEW=0)
add_definitions(-DCHECK_REAL_PHI_LOOPS=0)
add_definitions(-DPRINT_BBINDEX=0)    # Non zero to print <index>: before <statement number>
add_definitions(-DPRINT_BACK_EDGES=0) # Non zero to generate green back edges
add_definitions(-DDEBUG_SIMP=0)       # Set to 1 to print every change
add_definitions(-DDEBUG_PARAMS=1)     #
add_definitions(-DRECURSION_WIP=0)
add_definitions(-DPRINT_UNION=0)      # Set to 1 to debug unions to stderr
add_definitions(-DV9_ONLY=0)
add_definitions(-DBRANCH_DS_ERROR=0)  # If set, a branch to the delay slot of a delayed
                                      # CTI instruction is flagged as an error


###############################################################################
# Compiler specific flags section
###############################################################################

# Force C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_COMPILE_FLAGS "")
set(C_COMPILE_FLAGS "")
set(CXX_COMPILE_FLAGS "")
set(LINKER_FLAGS "")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS
    )
    set(DEBUG_LIB Dbghelp.lib)

    list(APPEND COMMON_COMPILE_FLAGS /W3)

elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # Warning options
    list(APPEND COMMON_COMPILE_FLAGS
        -Wall -Wextra -Werror
        -Weverything
        -Wno-documentation
        -Wno-documentation-unknown-command
        -Wno-unreachable-code-break
        -Wno-error=strict-aliasing
        -Wno-unused-local-typedefs
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-old-style-cast
        -Wno-reserved-id-macro
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-padded
        -Wno-weak-vtables
        -Wno-sign-conversion
        -Wno-cast-align
        -Wno-shorten-64-to-32
        -Wno-missing-prototypes
        -Wno-documentation-deprecated-sync
        -Wno-unused-macros
        -Wno-conversion
        -Wno-switch-enum
        -Wno-missing-variable-declarations
        -Wno-covered-switch-default
        -Wno-deprecated
        -Wno-double-promotion
        -Wno-float-equal
        -Wno-undefined-reinterpret-cast
        -Wno-implicit-fallthrough
        -Wno-missing-noreturn
        -Wno-gnu-zero-variadic-macro-arguments
        -Wno-undef
    )

elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    # Reject GCC < 4.9
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        message(FATAL_ERROR "Your GCC version is too old.\n\
            Please upgrade GCC or select another compiler.")
    endif ()

    list(APPEND COMMON_COMPILE_FLAGS
        -Wall -Wextra -Werror
        -Werror=pedantic
        -Wformat=2
        -Wmissing-include-dirs
        -rdynamic
        -fPIC
        -Wno-unused-local-typedefs
        -Wno-strict-aliasing
    )

    # other warnings:
    #-fstrict-aliasing  # This will not work with *reinterpret-cast<float*>(&int)
    #-Wsuggest-attribute=const|pure|etc. -Wcast-qual -Wconversion # TODO
    #-Wundef -Wzero-as-null-pointer-constant #does not compile with boost

    # Enable additional warnings for higher versions of GCC
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0)
        list(APPEND COMMON_COMPILE_FLAGS -Wformat-signedness)
        list(APPEND CXX_COMPILE_FLAGS
            -Wsuggest-override
            -Wshadow
            -Wstrict-overflow=2
            -Wno-error=strict-overflow
        )

        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
            list(APPEND COMMON_COMPILE_FLAGS
                -Wnull-dereference -Wduplicated-cond)
                #-Walloc-zero -Walloca -Wduplicated-branches
        endif ()
    endif ()

    list(APPEND LINKER_FLAGS -rdynamic)
endif ()


if (NOT MSVC)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        list(APPEND COMMON_COMPILE_FLAGS -g -O0)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
        list(APPEND COMMON_COMPILE_FLAGS -g -O2)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
        list(APPEND COMMON_COMPILE_FLAGS -Os)
    else () # Release
        list(APPEND COMMON_COMPILE_FLAGS -O3)
    endif ()
endif (NOT MSVC)

string(REPLACE ";" " " COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS}")
string(REPLACE ";" " " C_COMPILE_FLAGS      "${C_COMPILE_FLAGS}")
string(REPLACE ";" " " CXX_COMPILE_FLAGS    "${CXX_COMPILE_FLAGS}")

set(CMAKE_C_FLAGS          "${COMMON_COMPILE_FLAGS} ${C_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS        "${COMMON_COMPILE_FLAGS} ${CXX_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}")


###############################################################################
# Libraries section
###############################################################################

# Find Qt
find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Threads)

set(CMAKE_AUTOMOC ON)
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/")


###############################################################################
# Code section
###############################################################################

add_subdirectory(docs)
add_subdirectory(src)


###############################################################################
# Tests section
###############################################################################

if (BOOMERANG_BUILD_TESTS)
    enable_testing()
    find_package(Qt5Test REQUIRED)

    add_definitions(-DBOOMERANG_TEST_BASE="${BOOMERANG_OUTPUT_DIR}/")
    add_subdirectory(unit-tests)

    file(COPY "${CMAKE_SOURCE_DIR}/tests/inputs/" DESTINATION "${BOOMERANG_OUTPUT_DIR}/tests/inputs/")
endif (BOOMERANG_BUILD_TESTS)


# run regression suite by 'make check'
add_custom_target(check "./full_regression.sh"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

###############################################################################
# Additional targets section
###############################################################################

# copy function signatures to output directory
file(COPY "${CMAKE_SOURCE_DIR}/signatures/" DESTINATION "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/signatures/")
file(COPY "${CMAKE_SOURCE_DIR}/frontend/"   DESTINATION "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/frontend/")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/signatures" DESTINATION "lib/boomerang")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/frontend"   DESTINATION "lib/boomerang")

# delete all files in the 'out/' directory on make clean
set(EXTRA_CLEAN_FILES "${BOOMERANG_OUTPUT_DIR}")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${EXTRA_CLEAN_FILES}")

