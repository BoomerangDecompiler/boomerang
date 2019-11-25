#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

BOOMERANG_ADD_TEST(
    NAME BinaryImageTest
    SOURCES binary/BinaryImageTest.h binary/BinaryImageTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME BinarySectionTest
    SOURCES binary/BinarySectionTest.h binary/BinarySectionTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME BinarySymbolTableTest
    SOURCES binary/BinarySymbolTableTest.h binary/BinarySymbolTableTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME BinarySymbolTest
    SOURCES binary/BinarySymbolTest.h binary/BinarySymbolTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME LibProcTest
    SOURCES proc/LibProcTest.h proc/LibProcTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
    DEPENDENCIES
        boomerang-X86FrontEnd
        boomerang-ElfLoader
)


BOOMERANG_ADD_TEST(
    NAME BasicBlockTest
    SOURCES BasicBlockTest.h BasicBlockTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME DataFlowTest
    SOURCES DataFlowTest.h DataFlowTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
    DEPENDENCIES
        boomerang-ElfLoader
        boomerang-X86FrontEnd
)


BOOMERANG_ADD_TEST(
    NAME GlobalTest
    SOURCES GlobalTest.h GlobalTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
    DEPENDENCIES
        boomerang-ElfLoader
)


# BOOMERANG_ADD_TEST(
#     NAME IRFragmentTest
#     SOURCES IRFragmentTest.h IRFragmentTest.cpp
#     LIBRARIES
#         ${DEBUG_LIB}
#         boomerang
#         ${CMAKE_THREAD_LIBS_INIT}
# )


# BOOMERANG_ADD_TEST(
#     NAME ProcCFGTest
#     SOURCES proc/ProcCFGTest.h proc/ProcCFGTest.cpp
#     LIBRARIES
#         ${DEBUG_LIB}
#         boomerang
#         ${CMAKE_THREAD_LIBS_INIT}
# )


# BOOMERANG_ADD_TEST(
#     NAME UserProcTest
#     SOURCES proc/UserProcTest.h proc/UserProcTest.cpp
#     LIBRARIES
#         ${DEBUG_LIB}
#         boomerang
#         ${CMAKE_THREAD_LIBS_INIT}
#     DEPENDENCIES
#         boomerang-X86FrontEnd
#         boomerang-ElfLoader
# )


BOOMERANG_ADD_TEST(
    NAME SignatureTest
    SOURCES signature/SignatureTest.h signature/SignatureTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


# BOOMERANG_ADD_TEST(
#     NAME ProgTest
#     SOURCES ProgTest.h ProgTest.cpp
#     LIBRARIES
#         ${DEBUG_LIB}
#         boomerang
#         ${CMAKE_THREAD_LIBS_INIT}
#         boomerang-X86FrontEnd
#     DEPENDENCIES
#         boomerang-X86FrontEnd
#         boomerang-ElfLoader
# )
