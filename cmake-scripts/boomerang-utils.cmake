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

        install(FILES "${${SanitizedName}-Debug}" DESTINATION bin/ CONFIGURATIONS Debug)
        install(FILES "${${SanitizedName}-Release}" DESTINATION bin/ CONFIGURATIONS Release)
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
        set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")

        if (MSVC)
            # Visual Studio generates lib files for import in addition to dll files.
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/loader/")
        endif (MSVC)

        target_link_libraries(${target_name} Qt5::Core boomerang ${LOADER_LIBRARIES})
        target_compile_definitions(${target_name} PRIVATE BOOMERANG_BUILD_PLUGIN=1)

        install(TARGETS ${target_name}
            LIBRARY DESTINATION lib/boomerang/plugins/loader/
            RUNTIME DESTINATION lib/boomerang/plugins/loader/
        )
    endif (BOOMERANG_BUILD_LOADER_${LOADER_NAME})
endfunction()


#
# Usage: BOOMERANG_ADD_CODEGEN(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_CODEGEN)
    cmake_parse_arguments(CODEGEN "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    option(BOOMERANG_BUILD_CODEGEN_${CODEGEN_NAME} "Build the ${CODEGEN_NAME} code generator." ON)

    if (BOOMERANG_BUILD_CODEGEN_${CODEGEN_NAME})
        set(target_name "boomerang-${CODEGEN_NAME}Codegen")
        add_library(${target_name} SHARED ${CODEGEN_SOURCES})

        set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
        set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")

        if (MSVC)
            # Visual Studio generates lib files for import in addition to dll files.
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/codegen/")
        endif (MSVC)

        target_link_libraries(${target_name} Qt5::Core boomerang ${CODEGEN_LIBRARIES})
        target_compile_definitions(${target_name} PRIVATE BOOMERANG_BUILD_PLUGIN=1)

        install(TARGETS ${target_name}
            LIBRARY DESTINATION lib/boomerang/plugins/codegen/
            RUNTIME DESTINATION lib/boomerang/plugins/codegen/
        )
    endif (BOOMERANG_BUILD_CODEGEN_${CODEGEN_NAME})
endfunction()


#
# Usage: BOOMERANG_ADD_TYPERECOVERY(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_TYPERECOVERY)
    cmake_parse_arguments(TYPERECOVERY "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    option(BOOMERANG_BUILD_TYPERECOVERY_${TYPERECOVERY_NAME} "Build the ${TYPERECOVERY_NAME} type recovery engine." ON)

    if (BOOMERANG_BUILD_TYPERECOVERY_${TYPERECOVERY_NAME})
        set(target_name "boomerang-${TYPERECOVERY_NAME}TypeRecovery")
        add_library(${target_name} SHARED ${TYPERECOVERY_SOURCES})

        set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
        set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")

        if (MSVC)
            # Visual Studio generates lib files for import in addition to dll files.
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/type/")
        endif (MSVC)

        target_link_libraries(${target_name} Qt5::Core boomerang ${TYPERECOVERY_LIBRARIES})
        target_compile_definitions(${target_name} PRIVATE BOOMERANG_BUILD_PLUGIN=1)

        install(TARGETS ${target_name}
            LIBRARY DESTINATION lib/boomerang/plugins/type/
            RUNTIME DESTINATION lib/boomerang/plugins/type/
        )
    endif (BOOMERANG_BUILD_TYPERECOVERY_${TYPERECOVERY_NAME})
endfunction()


#
# Usage: BOOMERANG_ADD_SYMBOLPROVIDER(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_SYMBOLPROVIDER)
    cmake_parse_arguments(SYMBOLPROVIDER "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    option(BOOMERANG_BUILD_SYMBOLPROVIDER_${SYMBOLPROVIDER_NAME} "Build the ${SYMBOLPROVIDER_NAME} symbol provider." ON)

    if (BOOMERANG_BUILD_SYMBOLPROVIDER_${SYMBOLPROVIDER_NAME})
        set(target_name "boomerang-${SYMBOLPROVIDER_NAME}SymbolProvider")
        add_library(${target_name} SHARED ${SYMBOLPROVIDER_SOURCES})

        set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
        set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")

        if (MSVC)
            # Visual Studio generates lib files for import in addition to dll files.
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/symbol/")
        endif (MSVC)

        target_link_libraries(${target_name} Qt5::Core boomerang ${SYMBOLPROVIDER_LIBRARIES})
        target_compile_definitions(${target_name} PRIVATE BOOMERANG_BUILD_PLUGIN=1)

        install(TARGETS ${target_name}
            LIBRARY DESTINATION lib/boomerang/plugins/symbol/
            RUNTIME DESTINATION lib/boomerang/plugins/symbol/
        )
    endif (BOOMERANG_BUILD_SYMBOLPROVIDER_${SYMBOLPROVIDER_NAME})
endfunction()


#
# Usage: BOOMERANG_ADD_DECODER(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_DECODER)
    cmake_parse_arguments(DECODER "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    option(BOOMERANG_BUILD_DECODER_${DECODER_NAME} "Build the ${DECODER_NAME} decoder." ON)

    if (BOOMERANG_BUILD_DECODER_${DECODER_NAME})
        set(target_name "boomerang-${DECODER_NAME}Decoder")
        add_library(${target_name} SHARED ${DECODER_SOURCES})

        set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
        set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")

        if (MSVC)
            # Visual Studio generates lib files for import in addition to dll files.
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/decoder/")
        endif (MSVC)

        target_link_libraries(${target_name} Qt5::Core boomerang ${DECODER_LIBRARIES})
        target_compile_definitions(${target_name} PRIVATE BOOMERANG_BUILD_PLUGIN=1)

        install(TARGETS ${target_name}
            LIBRARY DESTINATION lib/boomerang/plugins/decoder/
            RUNTIME DESTINATION lib/boomerang/plugins/decoder/
        )
    endif (BOOMERANG_BUILD_DECODER_${DECODER_NAME})
endfunction()


#
# Usage: BOOMERANG_ADD_FRONTEND(NAME <name> SOURCES <source files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_FRONTEND)
    cmake_parse_arguments(FRONTEND "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    option(BOOMERANG_BUILD_FRONTEND_${FRONTEND_NAME} "Build the ${FRONTEND_NAME} front end." ON)

    if (BOOMERANG_BUILD_FRONTEND_${FRONTEND_NAME})
        set(target_name "boomerang-${FRONTEND_NAME}FrontEnd")
        add_library(${target_name} SHARED ${FRONTEND_SOURCES})

        set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
        set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")

        if (MSVC)
            # Visual Studio generates lib files for import in addition to dll files.
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
            set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
            set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BOOMERANG_OUTPUT_DIR}/lib/boomerang/plugins/frontend/")
        endif (MSVC)

        target_link_libraries(${target_name} Qt5::Core boomerang ${FRONTEND_LIBRARIES})
        target_compile_definitions(${target_name} PRIVATE BOOMERANG_BUILD_PLUGIN=1)

        install(TARGETS ${target_name}
            LIBRARY DESTINATION lib/boomerang/plugins/frontend/
            RUNTIME DESTINATION lib/boomerang/plugins/frontend/
        )
    endif (BOOMERANG_BUILD_FRONTEND_${FRONTEND_NAME})
endfunction()


#
# Usage: BOOMERANG_ADD_TEST(NAME <name> SOURCES <souce files> [ LIBRARIES <additional libs> ])
#
function(BOOMERANG_ADD_TEST)
    cmake_parse_arguments(TEST "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    get_filename_component(exename "${TEST_NAME}" NAME)

    add_executable(${exename} ${TEST_SOURCES})

    target_link_libraries(${exename}
        boomerang-test-utils
        ${TEST_LIBRARIES})

    add_test(NAME ${exename} COMMAND $<TARGET_FILE:${exename}>)
    set_property(TEST ${exename} APPEND PROPERTY ENVIRONMENT BOOMERANG_TEST_BASE=${BOOMERANG_OUTPUT_DIR})
    BOOMERANG_COPY_IMPORTED_DLL(${exename} Qt5::Test)
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


# Create a soft directory link from LINK_SOURCE to LINK_DEST
function(BOOMERANG_LINK_DIRECTORY LINK_SOURCE LINK_DEST)
    # link output data directory to ${CMAKE_SOURCE_DIR}/data"
    if (WIN32)
        string(REPLACE "/" "\\\\" LNK_LOC    "${LINK_SOURCE}")
        string(REPLACE "/" "\\\\" LNK_TARGET "${LINK_DEST}")

        # Do not invoke mklink directly. If invoked directly, mklink will fail if the link already exists.
        # But we only want to know if mklink fails because of some other reason.
        set(LNK_COMMAND "if not exist ${LNK_LOC} (mklink /J ${LNK_LOC} ${LNK_TARGET})")
        execute_process(COMMAND "cmd" /C "${LNK_COMMAND}"
            WORKING_DIRECTORY "${BOOMERANG_OUTPUT_DIR}"
            OUTPUT_QUIET
            ERROR_VARIABLE LNK_ERROR
            ERROR_STRIP_TRAILING_WHITESPACE
        )

        if (LNK_ERROR)
            message(WARNING "Could not link to data directory:\n"
                "Command '${LNK_COMMAND}' failed with\n"
                "error message '${LNK_ERROR}'")
        endif (LNK_ERROR)
    else () # Linux
        execute_process(COMMAND ln -sfn "${LINK_DEST}" "${LINK_SOURCE}")
    endif ()
endfunction()


# Append _SUFFIX to each elements of _LIST
function(BOOMERANG_LIST_APPEND_FOREACH _LIST _SUFFIX)
    set(list_temp "")
    foreach (elem ${${_LIST}})
        LIST(APPEND list_temp "${elem}${_SUFFIX}")
    endforeach ()
    set(${_LIST} ${list_temp} PARENT_SCOPE)
endfunction ()
