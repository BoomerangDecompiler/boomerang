
include(CMakeParseArguments)

#
# Usage: BOOMERANG_ADD_LOADER(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_LOADER)
	cmake_parse_arguments(LOADER "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

	option(BOOM_BUILD_LOADER_${LOADER_NAME} "Build the ${LOADER_NAME} loader." ON)

	if (BOOM_BUILD_LOADER_${LOADER_NAME})
		set(target_name "boomerang-${LOADER_NAME}Loader")
		add_library(${target_name} SHARED ${LOADER_SOURCES})

		set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
		if (WIN32)
			set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
		endif (WIN32)
		
		target_link_libraries(${target_name} Qt5::Core boomerang ${LOADER_LIBRARIES})

		install(TARGETS ${target_name}
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION lib
			RUNTIME DESTINATION bin
		)
	endif ()
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
# Usage: BOOMERANG_ADD_TEST(NAME <name> SOURCES <souce files> [ LIBRARIES <additional libs ])
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
endfunction(BOOMERANG_ADD_TEST)


#
# Copy files, but only if the sourc(es) differ from the destination.
# 
# Usage:
# BOOMERANG_COPY(
#     TARGET <tgt>
#     [PRE_BUILD|POST_BUILD]
#     [FILES <src_file(s)> | DIRECTORIES <src directories>]
#     DESTINATION <target dir or file>
# )
#
function(BOOMERANG_COPY)
	cmake_parse_arguments(CP "PRE_BUILD;POST_BUILD;RECURSIVE" "DESTINATION;TARGET" "FILES;DIRECTORIES" ${ARGN})
	
	if (NOT CP_FILES AND NOT CP_DIRECTORIES)
		message(AUTHOR_WARNING "You need to specify files or directories to copy.")
		return()
	endif ()
	
	if (CP_PRE_BUILD AND CP_POST_BUILD)
		message(AUTHOR_WARNING "You must not specify both PRE_BUILD and POST_BUILD options.")
		return()
	elseif (CP_PRE_BUILD)
		set(CP_RUN_TIME PRE_BUILD)
	elseif (CP_POST_BUILD)
		set(CP_RUN_TIME POST_BUILD)
	endif ()

	foreach (f ${CP_FILES})
		add_custom_command(
			TARGET ${CP_TARGET}
			${CP_RUN_TIME}
			COMMAND ${CMAKE_COMMAND}
			ARGS -E copy_if_different ${f} ${CP_DESTINATION}
		)
	endforeach ()
endfunction(BOOMERANG_COPY)
