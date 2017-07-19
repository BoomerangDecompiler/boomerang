
find_package(Doxygen)
if (Doxygen_FOUND)
	option(BOOM_BUILD_DOCS "Build the documentation" ON)
	if (BOOM_BUILD_DOCS)
		configure_file(
			${PROJECT_SOURCE_DIR}/docs/Doxyfile.in
			${PROJECT_BINARY_DIR}/docs/Doxyfile
			@ONLY
		)

		file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/)
		add_custom_target(docs
			COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			SOURCES mainpage.dox)

	endif(BOOM_BUILD_DOCS)
endif (Doxygen_FOUND)

