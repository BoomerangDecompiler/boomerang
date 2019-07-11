#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


if (BOOMERANG_BUILD_UNIT_TESTS)
    enable_testing()
    find_package(Qt5 COMPONENTS Test REQUIRED HINTS $ENV{QTDIR})
    if (Qt5_FOUND)
        mark_as_advanced(Qt5_DIR Qt5Test_DIR)
    endif (Qt5_FOUND)

    add_definitions(-DBOOMERANG_TEST_BASE="${BOOMERANG_OUTPUT_DIR}/")
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/unit-tests)
endif (BOOMERANG_BUILD_UNIT_TESTS)


if (BOOMERANG_BUILD_REGRESSION_TESTS)
    find_package(PythonInterp 3 REQUIRED)

    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/regression-tests)

    BOOMERANG_LINK_DIRECTORY(
        "${CMAKE_BINARY_DIR}/tests/regression-tests/expected-outputs"
        "${CMAKE_SOURCE_DIR}/tests/regression-tests/expected-outputs"
    )
endif (BOOMERANG_BUILD_REGRESSION_TESTS)
