#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


if (BOOMERANG_BUILD_TESTS)
    enable_testing()
    find_package(Qt5Test REQUIRED)
    if (Qt5Test_FOUND)
        mark_as_advanced(Qt5Test_DIR)
    endif (Qt5Test_FOUND)

    add_definitions(-DBOOMERANG_TEST_BASE="${BOOMERANG_OUTPUT_DIR}/")

    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/unit-tests)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/regression-tests)
endif (BOOMERANG_BUILD_TESTS)
