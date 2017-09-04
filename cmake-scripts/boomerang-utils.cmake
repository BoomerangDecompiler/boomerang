
include(CMakeParseArguments)


#
# Copy the Debug and Release DLL(s) for an IMPORTED target to the output directory on Windows
# Example: BOOMERANG_COPY_IMPORTED_DLL(Qt5::Core)
#
function(BOOMERANG_COPY_IMPORTED_DLL TargetName ImportedName)
    if (MSVC)
        string(REPLACE "::" "" SanitizedName "${ImportedName}")
        get_target_property(${SanitizedName}-Debug   "${ImportedName}" LOCATION_Debug)
        get_target_property(${SanitizedName}-Release "${ImportedName}" LOCATION_Release)

        add_custom_command(TARGET ${TargetName} POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            ARGS
                -E copy_if_different
                "${${SanitizedName}-Debug}"
                "${BOOMERANG_OUTPUT_DIR}/bin/$<CONFIG>/"
        )
        
        add_custom_command(TARGET ${TargetName} POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            ARGS
                -E copy_if_different
                "${${SanitizedName}-Release}"
                "${BOOMERANG_OUTPUT_DIR}/bin/$<CONFIG>/"
        )
    endif (MSVC)
endfunction(BOOMERANG_COPY_IMPORTED_DLL)


#
# Usage: BOOMERANG_ADD_LOADER(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_LOADER)
	cmake_parse_arguments(LOADER "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

	option(BOOMERANG_BUILD_LOADER_${LOADER_NAME} "Build the ${LOADER_NAME} loader." ON)

	if (BOOMERANG_BUILD_LOADER_${LOADER_NAME})
		set(target_name "boomerang-${LOADER_NAME}Loader")
		add_library(${target_name} SHARED ${LOADER_SOURCES})

		set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
		if (WIN32)
			set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
		endif (WIN32)

		target_link_libraries(${target_name} Qt5::Core boomerang ${LOADER_LIBRARIES})

		install(TARGETS ${target_name}
			ARCHIVE DESTINATION lib/boomerang/plugins/loader/
			LIBRARY DESTINATION lib/boomerang/plugins/loader/
		)
	endif (BOOMERANG_BUILD_LOADER_${LOADER_NAME})
endfunction()


#
# Usage: BOOMERANG_ADD_FRONTEND(NAME <name> [ SOURCES <source files> ] [ LIBRARIES <additional libs> ])
#
# Note: There must be a <name>decoder.h/.cpp and a <name>frontend.h/.cpp present
# in the same directory as the CMakeLists.txt where this function is invoked.
#
function(BOOMERANG_ADD_FRONTEND)
	cmake_parse_arguments(FRONTEND "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

	# add the frontend as a static library
	add_library(boomerang-${FRONTEND_NAME}Frontend STATIC
		${FRONTEND_NAME}decoder.h
		${FRONTEND_NAME}decoder.cpp
		${FRONTEND_NAME}frontend.h
		${FRONTEND_NAME}frontend.cpp
		${FRONTEND_SOURCES}
	)

	target_link_libraries(${FRONTEND_NAME} Qt5::Core ${FRONTEND_LIBRARIES})
endfunction(BOOMERANG_ADD_FRONTEND)


#
# Usage: BOOMERANG_ADD_CODEGEN(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_CODEGEN)
	cmake_parse_arguments(CODEGEN "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

	add_library(boomerang-${CODEGEN_NAME}Codegen STATIC
		${CODEGEN_SOURCES}
	)

	target_link_libraries(boomerang-${CODEGEN_NAME}Codegen Qt5::Core ${CODEGEN_LIBRARIES})
endfunction(BOOMERANG_ADD_CODEGEN)


#
# Usage: BOOMERANG_ADD_TEST(NAME <name> SOURCES <souce files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_TEST)
	cmake_parse_arguments(TEST "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

	add_executable(${TEST_NAME} ${TEST_SOURCES})
	target_link_libraries(${TEST_NAME}
		Qt5::Core
		Qt5::Test
		${TEST_LIBRARIES})

	add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
	set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT BOOMERANG_TEST_BASE=${BOOMERANG_OUTPUT_DIR})
    BOOMERANG_COPY_IMPORTED_DLL(${TEST_NAME} Qt5::Test)
endfunction(BOOMERANG_ADD_TEST)
