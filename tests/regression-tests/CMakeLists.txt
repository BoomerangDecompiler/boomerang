
# regression tests are executed via the CLI
if (BOOMERANG_BUILD_TESTS AND BOOMERANG_BUILD_CLI)
    add_custom_command(OUTPUT copy-regression-script
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/regression_tester.py ${CMAKE_CURRENT_BINARY_DIR}/
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/regression_tester.py"
    )

    # run regression suite by 'make check'
    add_custom_target(check
        "${CMAKE_CURRENT_SOURCE_DIR}/full_regression.sh" "$<TARGET_FILE:boomerang-cli>"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
        DEPENDS copy-regression-script
    )

    add_dependencies(check
        boomerang-DOS4GWLoader
        boomerang-ElfLoader
        boomerang-ExeLoader
        boomerang-HpSomLoader
        boomerang-MachOLoader
        boomerang-PalmLoader
        boomerang-Win32Loader
    )
endif (BOOMERANG_BUILD_TESTS AND BOOMERANG_BUILD_CLI)
