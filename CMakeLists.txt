cmake_minimum_required(VERSION 3.1)
project(Boomerang VERSION 0.4.0 LANGUAGES C CXX)
set(CMAKE_WARN_DEPRECATED ON)

###############################################################################
# Settings related to build system, and target directories
###############################################################################

# if git is installed, update version string from git tags
find_package(Git)
if (GIT_FOUND)
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" describe --dirty --tags
		RESULT_VARIABLE DESCRIBE_FAILED
		OUTPUT_VARIABLE VERSION_TEMP
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if (NOT DESCRIBE_FAILED)
		set(PROJECT_VERSION   "${VERSION_TEMP}")
		set(BOOMERANG_VERSION "${VERSION_TEMP}")
	endif (NOT DESCRIBE_FAILED)
endif (GIT_FOUND)

message(STATUS "Configuring " ${PROJECT_NAME} " " ${BOOMERANG_VERSION})
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

# Set a single output directory for generated binaries.
# This is not ideal, but necessary for tests.
set(BOOMERANG_OUTPUT_DIR "${PROJECT_BINARY_DIR}/out")

set(CMAKE_SHARED_MODULE_PREFIX "") # prevent windows/mingw modules having lib* prefix
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BOOMERANG_OUTPUT_DIR}/lib")

if (WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif ()


###############################################################################
# Options section
###############################################################################
option(BOOM_BUILD_TESTS "Build the testing tree." ON)

# CMake does not fill in CMAKE_BUILD_TYPE automatically.
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type. Valid values are Debug Release MinSizeRel RelWithDebInfo")

# help the cmake find 3rd_party compiled/installed packages
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/3rd_party")


###############################################################################
# Compiler specific flags section
###############################################################################

set(COMMON_COMPILE_OPTIONS "")

if (MSVC)
    add_definitions(
    	-D_CRT_SECURE_NO_WARNINGS
    	-D_CRT_NONSTDC_NO_DEPRECATE
    	-D_SCL_SECURE_NO_WARNINGS
    )
    set(DEBUG_LIB Dbghelp.lib)
    
elseif (CMAKE_CXX_COMPILER MATCHES clang)
	set(COMMON_COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS} \
		-Wall -Wextra -Werror \
		-Wno-return-type-c-linkage \
		-Weverything \
		-Wno-c++98-compat \
		-Wno-c++98-compat-pedantic \
		-Wno-old-style-cast \
		-Wno-documentation-unknown-command \
		-Wno-reserved-id-macro \
		-Wno-exit-time-destructors \
		-Wno-global-constructors \
		-Wno-padded \
		-Wno-weak-vtables \
		-Wno-sign-conversion \
		-Wno-cast-align \
		-Wno-shorten-64-to-32 \
		-Wno-missing-prototypes \
		-Wno-unreachable-code \
		-Wno-unreachable-code-return \
		-Wno-unreachable-code-break \
		-Wno-documentation-deprecated-sync \
		-Wno-unused-macros \
		-Wno-conversion \
		-Wno-switch-enum \
		-Wno-missing-variable-declarations \
		-Wno-covered-switch-default \
		-Wno-deprecated \
		-Wno-double-promotion \
		-Wno-float-equal \
		-Wno-undefined-reinterpret-cast \
		-Wno-implicit-fallthrough \
		-Wno-missing-noreturn \
		-Wno-gnu-zero-variadic-macro-arguments")
else ()
	set(COMMON_COMPILE_OPTIONS "-Wall -Wextra -Werror -Werror=pedantic \
		-Wshadow -Wformat=2 -Wformat-signedness -Wmissing-include-dirs \
		-Wstrict-overflow=2 -rdynamic -fPIC"
	)
	
	# other warnings:
	#-fstrict-aliasing  # This will not work with *reinterpret-cast<float*>(&int)
    #-Wsuggest-attribute=const|pure|etc. -Wcast-qual -Wconversion # TODO
	#-Wundef -Wzero-as-null-pointer-constant #does not compile with boost
	
	# Enable additional warnings for GCC6+
	if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
		set(COMMON_COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS} \
			-Wnull-dereference -Wduplicated-cond")
			#-Walloc-zero -Walloca -Wduplicated-branches 
	endif ()

	set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=c++11 -Wsuggest-override")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
    add_definitions(-D__STDC_FORMAT_MACROS)
endif ()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${COMMON_COMPILE_OPTIONS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COMPILE_OPTIONS}")


###############################################################################
# Libraries section
###############################################################################

# Find Qt
find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Threads)

set(CMAKE_AUTOMOC ON)


###############################################################################
# Defines/configuration section
###############################################################################

add_definitions(-DBOOMERANG_VERSION="${BOOMERANG_VERSION}")
include(boomerang-configure)

# Check for big/little endian
if (WORDS_BIGENDIAN)
	add_definitions(-DBOOMERANG_BIG_ENDIAN=1)
else ()
	add_definitions(-DBOOMERANG_BIG_ENDIAN=0)
endif ()

# Check 32/64 bit system
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	add_definitions(-DBOOMERANG_BIT_COUNT=64)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
	add_definitions(-DBOOMERANG_BIT_COUNT=32)
else ()
	message(FATAL_ERROR "Unknown platform with sizeof(void*) == " ${CMAKE_SIZEOF_VOID_P})
endif ()

# Define this to 1 if you want to use Dominance Numbers for analysis
add_definitions(-DUSE_DOMINANCE_NUMS=0)
add_definitions(-DBCCTR_LONG=0)
add_definitions(-DYYMAXDEPTH=10000)
add_definitions(-DSYMS_IN_BACK_END=0)
add_definitions(-DDEBUG_SPLIT_FOR_BRANCH=0)
add_definitions(-DNEW=0)

###############################################################################
# Code section
###############################################################################

add_subdirectory(docs)
add_subdirectory(src)


###############################################################################
# Tests section
###############################################################################

if (BOOM_BUILD_TESTS)
    enable_testing()
    find_package(Qt5Test REQUIRED)
    
	add_subdirectory(unit-tests)
endif (BOOM_BUILD_TESTS)


# run regression suite by 'make check'
add_custom_target(check "./full_regression.sh"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

###############################################################################
# Additional targets section
###############################################################################

# copy function signatures and test files to output directory
add_custom_target(COPY_FILES ALL)
add_custom_command(TARGET COPY_FILES POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/signatures ${BOOMERANG_OUTPUT_DIR}/signatures)
add_custom_command(TARGET COPY_FILES POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/inputs ${BOOMERANG_OUTPUT_DIR}/tests/inputs)

# delete all files in the 'out/' directory on make clean
set(EXTRA_CLEAN_FILES "${BOOMERANG_OUTPUT_DIR}")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${EXTRA_CLEAN_FILES}")

