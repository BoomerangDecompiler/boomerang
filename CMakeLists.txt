cmake_minimum_required (VERSION 2.8)
project (boomerang)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
   message(SEND_ERROR "In-source builds are not allowed! mkdir build, cd build, cmake ..")
endif()

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)
INCLUDE(TestBigEndian)
INCLUDE(FindEXPAT)

MACRO(SET_01 A B)
	IF(${B})
		SET(${A} 1)
	ELSE(${B})
		SET(${A} 0)
	ENDIF(${B})
ENDMACRO(SET_01 A B)

MACRO(SET_01_2 A B C)
	IF(${B} AND ${C})
		SET(${A} 1)
	ELSE(${B} AND ${C})
		SET(${A} 0)
	ENDIF(${B} AND ${C})
ENDMACRO(SET_01_2 A B C)

# -ldl (dlopen)
CHECK_INCLUDE_FILES(dlfcn.h            DL_H)
CHECK_LIBRARY_EXISTS(dl dlopen ""      DL_L)
SET_01_2(DL_FOUND DL_H DL_L)

IF(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	SET(DL_LIBS -ldl)
ELSE(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	SET(DL_LIBS)
ENDIF(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")

# CppUnit
FIND_PROGRAM(CPPUNIT_CONFIG_EXECUTABLE NAMES cppunit-config PATHS /usr/local/bin)
IF(CPPUNIT_CONFIG_EXECUTABLE)
	EXEC_PROGRAM(${CPPUNIT_CONFIG_EXECUTABLE} ARGS --cflags OUTPUT_VARIABLE CPPUNIT_CFLAGS)
	EXEC_PROGRAM(${CPPUNIT_CONFIG_EXECUTABLE} ARGS --libs OUTPUT_VARIABLE CPPUNIT_LIBS)
	SET(CPPUNIT_FOUND 1)
ENDIF(CPPUNIT_CONFIG_EXECUTABLE)

# GC
CHECK_INCLUDE_FILES(gc.h               GC_H)
CHECK_LIBRARY_EXISTS(gc GC_malloc ""   GC_L)
SET_01_2(GC_FOUND GC_H GC_L)
SET(GC_LIBS -lgc)

# Expat
FIND_PACKAGE(EXPAT)

#set variables to expand in config.h.cmake
SET_01(HAVE_LIBCPPUNIT    CPPUNIT_FOUND)
SET_01(HAVE_LIBDL         DL_FOUND)
SET_01(HAVE_LIBEXPAT      EXPAT_FOUND)
SET_01(HAVE_LIBGC         GC_FOUND)
CHECK_INCLUDE_FILES(inttypes.h    HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES(memory.h      HAVE_MEMORY_H)
CHECK_INCLUDE_FILES(stdint.h      HAVE_STDINT_H)
CHECK_INCLUDE_FILES(stdlib.h      HAVE_STDLIB_H)
CHECK_INCLUDE_FILES(strings.h     HAVE_STRINGS_H)
CHECK_INCLUDE_FILES(string.h      HAVE_STRING_H)
CHECK_INCLUDE_FILES(sys/stat.h    HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES(sys/types.h   HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES(unistd.h      HAVE_UNISTD_H)
IF("${CMAKE_SYSTEM_NAME}" STREQUAL "MacOS")
	SET(HOST_OSX 1)
ENDIF("${CMAKE_SYSTEM_NAME}" STREQUAL "MacOS")
SET(PACKAGE_BUGREPORT "http://boomerang.sourceforge.net")
SET(PACKAGE_NAME      "boomerang")
SET(PACKAGE_STRING    "boomerang")
SET(PACKAGE_TARNAME   "boomerang")
SET(PACKAGE_VERSION   "0.3.99")
CHECK_TYPE_SIZE("char"      SIZEOF_CHAR)
CHECK_TYPE_SIZE("double"    SIZEOF_DOUBLE)
CHECK_TYPE_SIZE("float"     SIZEOF_FLOAT)
CHECK_TYPE_SIZE("int"       SIZEOF_INT)
CHECK_TYPE_SIZE("int*"      SIZEOF_INT_P)
CHECK_TYPE_SIZE("long"      SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("short"     SIZEOF_SHORT)
TEST_BIG_ENDIAN(BIG_ENDIAN)
SET_01(WORDS_BIGENDIAN BIG_ENDIAN)

# generate config.h
CONFIGURE_FILE(include/config.h.cmake include/config.h)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/include")
INCLUDE_DIRECTORIES("${EXPAT_INCLUDE_DIRS}")

LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/loader")

MACRO(ADDCFLAG SRCFILE FLAG)
	GET_SOURCE_FILE_PROPERTY(loc_flags "${SRCFILE}" COMPILE_FLAGS)
	IF(loc_flags)
		SET_SOURCE_FILES_PROPERTIES("${SRCFILE}" PROPERTIES   COMPILE_FLAGS "${loc_flags} ${FLAG}")
	ELSE(loc_flags)
		SET_SOURCE_FILES_PROPERTIES("${SRCFILE}" PROPERTIES   COMPILE_FLAGS "${FLAG}")
	ENDIF(loc_flags)
	SET(loc_flags)
ENDMACRO(ADDCFLAG SRCFILE FLAG)

ADDCFLAG(db/prog.cpp "-I${CMAKE_CURRENT_SOURCE_DIR}/codegen")
ADDCFLAG(db/prog.cpp "-I${CMAKE_CURRENT_SOURCE_DIR}/c")
ADDCFLAG(frontend/frontend.cpp "-I${CMAKE_CURRENT_SOURCE_DIR}/c")
ADDCFLAG(frontend/pentiumdecoder.cpp -fno-exceptions)

SET(boomerang_SRC
	boomerang.cpp
	driver.cpp
	log.cpp
	c/ansi-c-parser.cpp
	c/ansi-c-scanner.cpp
	codegen/chllcode.cpp
	codegen/syntax.cpp
	db/basicblock.cpp
	db/cfg.cpp
	db/dataflow.cpp
	db/exp.cpp
	db/insnameelem.cpp
	db/managed.cpp
	db/proc.cpp
	db/prog.cpp #-Icodegen -Ic
	db/register.cpp
	db/rtl.cpp
	db/signature.cpp
	db/sslinst.cpp
	db/sslparser.cpp
	db/sslscanner.cpp
	db/statement.cpp
	db/table.cpp
	db/visitor.cpp
	db/xmlprogparser.cpp
	frontend/frontend.cpp #-Ic
	frontend/njmcDecoder.cpp
	frontend/pentiumdecoder.cpp #-fno-exceptions
	frontend/pentiumfrontend.cpp
	frontend/ppcdecoder.cpp
	frontend/ppcfrontend.cpp
	frontend/sparcdecoder.cpp
	frontend/sparcfrontend.cpp
	frontend/mipsdecoder.cpp
	frontend/mipsfrontend.cpp
	frontend/st20decoder.cpp
	frontend/st20frontend.cpp
	loader/BinaryFileFactory.cpp
	type/constraint.cpp
	type/dfa.cpp
	type/type.cpp
	util/util.cpp
)

ADD_EXECUTABLE(boomerang ${boomerang_SRC})

TARGET_LINK_LIBRARIES(boomerang ${DL_LIBS} ${GC_LIBS} "${EXPAT_LIBRARIES}" BinaryFile)

ADD_SUBDIRECTORY(loader)