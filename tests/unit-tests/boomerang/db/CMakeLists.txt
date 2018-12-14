#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

set(TESTS
    binary/BinaryImageTest
    binary/BinarySectionTest
    binary/BinarySymbolTableTest
    binary/BinarySymbolTest
    proc/LibProcTest
    proc/ProcCFGTest
    proc/UserProcTest
    signature/SignatureTest
    BasicBlockTest
    GlobalTest
    ProgTest
)

# These tests require the ELF loader
set(TESTS_WITH_ELF
    DataFlowTest
)

foreach(t ${TESTS})
    string(REGEX REPLACE ".*/" "" TEST_NAME ${t})
    BOOMERANG_ADD_TEST(
        NAME ${TEST_NAME}
        SOURCES ${t}.h ${t}.cpp
        LIBRARIES
            ${DEBUG_LIB}
            boomerang
            ${CMAKE_THREAD_LIBS_INIT}
    )
endforeach()

target_link_libraries(ProgTest
    boomerang-X86FrontEnd
)

if (MSVC)
    add_custom_command(TARGET ProgTest POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        ARGS
            -E copy_if_different
            "$<TARGET_FILE:boomerang-X86FrontEnd>"
            "${BOOMERANG_OUTPUT_DIR}/bin/"
    )
endif (MSVC)


if (BOOMERANG_BUILD_LOADER_Elf)
    foreach(t ${TESTS_WITH_ELF})
        BOOMERANG_ADD_TEST(
            NAME ${t}
            SOURCES ${t}.h ${t}.cpp
            LIBRARIES
                ${DEBUG_LIB}
                boomerang
                ${CMAKE_THREAD_LIBS_INIT}
        )
    endforeach()
endif (BOOMERANG_BUILD_LOADER_Elf)
