PROJECT(Boomerang)
include(ExternalProject)
###############################################################################
# Settings related to build system, and target directories
###############################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.10)
CMAKE_POLICY(SET CMP0020 OLD)
IF(POLICY CMP0043)
CMAKE_POLICY(SET CMP0043 OLD)
ENDIF()

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_scripts)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
IF(WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/out)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/out)
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
ENDIF()
###############################################################################
# Options section
###############################################################################
OPTION(BUILD_TESTING "Build the testing tree." ON)
# help the cmake find 3rd_party compiled/installed packages
set(CMAKE_PREFIX_PATH  "${CMAKE_PREFIX_PATH};${PROJECT_SOURCE_DIR}/3rd_party")

IF(BUILD_TESTING)
    enable_testing()
    find_package(Qt5Test)
    #find_package(Protobuf REQUIRED)
ENDIF(BUILD_TESTING)

###############################################################################
# Compiler specific flags section
###############################################################################

IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    message(SEND_ERROR "This is a c++11 build of boomerang it's highly likely it will fail under VisualStudio")
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11 -pedantic" )
    IF(CMAKE_CXX_COMPILER MATCHES clang)
        #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything  -Wno-padded -Wno-c++98-compat -Wno-weak-vtables -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-exit-time-destructors" )
    else()
        #add_definitions(-D_GLIBCXX_DEBUG) #-D_GLIBCXX_DEBUG
    endif()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra" )
    add_definitions(-D__STDC_FORMAT_MACROS) #-D_GLIBCXX_DEBUG
ENDIF()
###############################################################################
# Defines section
###############################################################################
SET(VERSION "alpha 0.3.99 xx/xxx/2014")
SET(MAIN_INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)
INCLUDE(TestBigEndian)

MACRO(SET_01 A B)
    IF(${B})
        SET(${A} 1)
    ELSE(${B})
        SET(${A} 0)
    ENDIF(${B})
ENDMACRO(SET_01 A B)

find_package(Boost REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/3rd_party/include
)
#set variables to expand in config.h.cmake
SET_01(HAVE_LIBCPPUNIT    CPPUNIT_FOUND)
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
SET(PACKAGE_BUGREPORT "http://github.com/nemerle/boomerang")
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

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}/include"
        ${Qt5Core_INCLUDE_DIRS}
)
link_directories(
    ${PROJECT_SOURCE_DIR}/3rd_party/lib
    "${CMAKE_CURRENT_BINARY_DIR}/loader"
)

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

ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(c)
ADD_SUBDIRECTORY(codegen)
ADD_SUBDIRECTORY(db)
ADD_SUBDIRECTORY(frontend)
ADD_SUBDIRECTORY(type)
ADD_SUBDIRECTORY(transform)
IF(BUILD_TESTING)
ADD_SUBDIRECTORY(unit_testing)
ENDIF()
SET(boom_base_SRC
        loader/BinaryFileFactory.cpp
        boomerang.cpp
        log.cpp
)
IF(MSVC)
    SET(DEBUG_LIB Dbghelp.lib )
ENDIF()
ADD_LIBRARY(boom_base ${boom_base_SRC} )
qt5_use_modules(boom_base Core Xml)

ADD_SUBDIRECTORY(loader)
ADD_SUBDIRECTORY(ui)
