
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/regression_tester.py" DESTINATION "${BOOMERANG_OUTPUT_DIR}/")

# run regression suite by 'make check'
add_custom_target(check
    "${CMAKE_CURRENT_SOURCE_DIR}/full_regression.sh" "$<TARGET_FILE:boomerang-cli>"
    WORKING_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/"
)
