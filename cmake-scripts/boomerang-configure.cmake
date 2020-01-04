#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# This script will perform configuration of all system specific settings

# Boomerang configuration options
option(BOOMERANG_BUILD_GUI              "Build the GUI. Requires Qt5Widgets." ON)
option(BOOMERANG_BUILD_CLI              "Build the command line interface." ON)
option(BOOMERANG_BUILD_UNIT_TESTS       "Build the unit tests. Requires Qt5Test." OFF)

if (BOOMERANG_BUILD_CLI)
    option(BOOMERANG_BUILD_REGRESSION_TESTS "Build the regression tests. Requires Python 3." OFF)
endif (BOOMERANG_BUILD_CLI)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    option(BOOMERANG_ENABLE_COVERAGE "Build with coverage compiler flags enabled." OFF)
endif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

option(BOOMERANG_INSTALL_SAMPLES "Install sample binaries." OFF)


# Check for big/little endian
include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

if (WORDS_BIGENDIAN)
    add_definitions(-DBOOMERANG_BIG_ENDIAN=1)
else ()
    add_definitions(-DBOOMERANG_BIG_ENDIAN=0)
endif ()


# Check 32/64 bit system
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DBOOMERANG_BITNESS=64)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "Compiling Boomerang as a 32 bit binary is not officially supported. "
            "Please consider compiling Boomerang as a 64 bit binary.")
    add_definitions(-DBOOMERANG_BITNESS=32)
else ()
    message(FATAL_ERROR "Unknown platform with sizeof(void*) == ${CMAKE_SIZEOF_VOID_P}")
endif ()


add_definitions(-DDEBUG=0)
add_definitions(-DDEBUG_PARAMS=1)
