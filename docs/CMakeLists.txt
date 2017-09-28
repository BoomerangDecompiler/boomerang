#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


find_package(Doxygen)

if (Doxygen_FOUND)
	option(BOOMERANG_BUILD_DOCS "Build the documentation" ON)
	if (BOOMERANG_BUILD_DOCS)
		configure_file(
			${PROJECT_SOURCE_DIR}/docs/Doxyfile.in
			${PROJECT_BINARY_DIR}/docs/Doxyfile
			@ONLY
		)

		file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/)
		add_custom_target(docs
			COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			VERBATIM
			SOURCES mainpage.dox
		)
	endif (BOOMERANG_BUILD_DOCS)
endif (Doxygen_FOUND)

