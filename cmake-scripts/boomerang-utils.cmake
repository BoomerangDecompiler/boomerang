#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


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
                "${BOOMERANG_OUTPUT_DIR}/bin/"
        )

        add_custom_command(TARGET ${TargetName} POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            ARGS
                -E copy_if_different
                "${${SanitizedName}-Release}"
                "${BOOMERANG_OUTPUT_DIR}/bin/"
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

		set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
		if (WIN32)
			set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
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


include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
# This function adds the flag(s) to the c/c++ compiler flags
function(BOOMERANG_ADD_COMPILE_FLAGS)
    set(C_COMPILE_FLAGS "")
    set(CXX_COMPILE_FLAGS "")

    foreach (flag ${ARGN})
        # We cannot check for -Wno-foo as this won't throw a warning so we must check for the -Wfoo option directly
        # https://stackoverflow.com/questions/38785168/cc1plus-unrecognized-command-line-option-warning-on-any-other-warning
        string(REGEX REPLACE "^-Wno-" "-W" checkedFlag ${flag})
        set(VarName ${checkedFlag})
        string(REPLACE "+" "X" VarName ${VarName})
        string(REGEX REPLACE "[-=]" "_" VarName ${VarName})

        # Avoid double checks. A compiler will not magically support a flag it did not before
        if (NOT ${VarName}_CHECKED)
            CHECK_CXX_COMPILER_FLAG(${checkedFlag} CXX_FLAG_${VarName}_SUPPORTED)
            CHECK_C_COMPILER_FLAG(${checkedFlag}   C_FLAG_${VarName}_SUPPORTED)
            set(${VarName}_CHECKED YES CACHE INTERNAL "")
        endif()

        if (CXX_FLAG_${VarName}_SUPPORTED)
            set(CXX_COMPILE_FLAGS "${C_COMPILE_FLAGS} ${flag}")
        endif ()
        if (C_FLAG_${VarName}_SUPPORTED)
            set(C_COMPILE_FLAGS "${C_COMPILE_FLAGS} ${flag}")
        endif ()

        unset(VarName)
        unset(checkedFlag)
    endforeach ()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_COMPILE_FLAGS}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${C_COMPILE_FLAGS}" PARENT_SCOPE)
endfunction()