PROJECT(Boomerang)
include(ExternalProject)
###############################################################################
# Settings related to build system, and target directories
###############################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
CMAKE_POLICY(SET CMP0020 OLD)
IF(POLICY CMP0043)
CMAKE_POLICY(SET CMP0043 OLD)
ENDIF()

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_scripts)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_SHARED_MODULE_PREFIX ) # prevent windows/mingw modules having lib* prefix

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
IF(MSVC)
    SET(DEBUG_LIB Dbghelp.lib )
ENDIF()

INCLUDE(TestBigEndian)
find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Threads REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

###############################################################################
# Defines section
###############################################################################
SET(VERSION "alpha 0.3.99 xx/xxx/2016")
SET(MAIN_INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
SET(PACKAGE_BUGREPORT "http://github.com/nemerle/boomerang")
SET(PACKAGE_NAME      "boomerang")
SET(PACKAGE_STRING    "boomerang")
SET(PACKAGE_TARNAME   "boomerang")
SET(PACKAGE_VERSION   "0.3.99")


# generate config.h
CONFIGURE_FILE(include/config.h.cmake include/config.h)

INCLUDE_DIRECTORIES(
    .
    "${PROJECT_SOURCE_DIR}/3rd_party"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    ${Qt5Core_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/3rd_party/include"
)
link_directories(
    "${PROJECT_SOURCE_DIR}/3rd_party/lib"
    "${CMAKE_CURRENT_BINARY_DIR}/loader"
)

ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(c)
ADD_SUBDIRECTORY(codegen)
ADD_SUBDIRECTORY(db)
ADD_SUBDIRECTORY(frontend)
ADD_SUBDIRECTORY(type)
ADD_SUBDIRECTORY(transform)
ADD_SUBDIRECTORY(passes)
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(unit_testing)
ENDIF()

SET(boom_base_SRC
        loader/BinaryFileFactory.cpp
        boomerang.cpp
        log.cpp
)
ADD_LIBRARY(boom_base ${boom_base_SRC} )
qt5_use_modules(boom_base Core Xml)

ADD_SUBDIRECTORY(loader)
ADD_SUBDIRECTORY(ui)
