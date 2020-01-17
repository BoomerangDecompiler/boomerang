#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)


BOOMERANG_ADD_TEST(
    NAME ExpTest
    SOURCES exp/ExpTest.h exp/ExpTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME ParserTest
    SOURCES parser/ParserTest.h parser/ParserTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME AssignTest
    SOURCES statements/AssignTest.h statements/AssignTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)

BOOMERANG_ADD_TEST(
    NAME BoolAssignTest
    SOURCES statements/BoolAssignTest.h statements/BoolAssignTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME StatementTest
    SOURCES statements/StatementTest.h statements/StatementTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
    DEPENDENCIES
        boomerang-ElfLoader
        boomerang-X86FrontEnd
)


BOOMERANG_ADD_TEST(
    NAME ArrayTypeTest
    SOURCES type/ArrayTypeTest.h type/ArrayTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME BooleanTypeTest
    SOURCES type/BooleanTypeTest.h type/BooleanTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME CharTypeTest
    SOURCES type/CharTypeTest.h type/CharTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME CompoundTypeTest
    SOURCES type/CompoundTypeTest.h type/CompoundTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME FloatTypeTest
    SOURCES type/FloatTypeTest.h type/FloatTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME FuncTypeTest
    SOURCES type/FuncTypeTest.h type/FuncTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME NamedTypeTest
    SOURCES type/NamedTypeTest.h type/NamedTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME IntegerTypeTest
    SOURCES type/IntegerTypeTest.h type/IntegerTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME PointerTypeTest
    SOURCES type/PointerTypeTest.h type/PointerTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME MeetTest
    SOURCES type/MeetTest.h type/MeetTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME TypeTest
    SOURCES type/TypeTest.h type/TypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
    DEPENDENCIES
        boomerang-Win32Loader
)


BOOMERANG_ADD_TEST(
    NAME UnionTypeTest
    SOURCES type/UnionTypeTest.h type/UnionTypeTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME RegDBTest
    SOURCES RegDBTest.h RegDBTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)


BOOMERANG_ADD_TEST(
    NAME RTLTest
    SOURCES RTLTest.h RTLTest.cpp
    LIBRARIES
        ${DEBUG_LIB}
        boomerang
        ${CMAKE_THREAD_LIBS_INIT}
)
