#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

if (BOOMERANG_BUILD_REGRESSION_TESTS)
    find_package(PythonInterp 3 REQUIRED)

    add_custom_command(OUTPUT copy-regression-script
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/regression-tester.py ${CMAKE_CURRENT_BINARY_DIR}/
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/regression-tester.py"
    )

    # run regression suite by 'make check'
    add_custom_target(check
        "${PYTHON_EXECUTABLE}" "./regression-tester.py" "$<TARGET_FILE:boomerang-cli>"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
        DEPENDS copy-regression-script
    )
endif (BOOMERANG_BUILD_REGRESSION_TESTS)
