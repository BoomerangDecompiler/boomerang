#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)



# These tests require the ELF loader
set(TESTS_WITH_ELF
    PentiumFrontEndTest
    SPARCFrontEndTest
)


if (BOOMERANG_BUILD_LOADER_Elf)
    foreach(t ${TESTS_WITH_ELF})
        BOOMERANG_ADD_TEST(
            NAME ${t}
            SOURCES ${t}.h ${t}.cpp
            LIBRARIES
                ${DEBUG_LIB}
                boomerang
                ${CMAKE_DL_LIBS}
                ${CMAKE_THREAD_LIBS_INIT}
        )
    endforeach()

    target_link_libraries(PentiumFrontEndTest boomerang-X86FrontEnd)
    target_link_libraries(SPARCFrontEndTest boomerang-SPARCFrontEnd)
endif (BOOMERANG_BUILD_LOADER_Elf)
