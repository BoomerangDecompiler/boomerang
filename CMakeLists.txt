cmake_minimum_required(VERSION 3.1)
project(Boomerang VERSION 0.4.0 LANGUAGES C CXX)

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

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_SHARED_MODULE_PREFIX "") # prevent windows/mingw modules having lib* prefix

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
if (WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/out)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/out)
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif ()

###############################################################################
# Options section
###############################################################################
option(BOOM_BUILD_TESTS "Build the testing tree." ON)

# CMake does not fill in CMAKE_BUILD_TYPE automatically.
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type. Valid values are Debug Release MinSizeRel RelWithDebInfo")

# help the cmake find 3rd_party compiled/installed packages
set(CMAKE_PREFIX_PATH  "${CMAKE_PREFIX_PATH};${PROJECT_SOURCE_DIR}/3rd_party")

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
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything  -Wno-padded -Wno-c++98-compat -Wno-weak-vtables -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-exit-time-destructors" )
	#add_definitions(-D_GLIBCXX_DEBUG) #-D_GLIBCXX_DEBUG
else ()
	set(COMMON_COMPILE_OPTIONS "-Wall -Wextra -Werror -Werror=pedantic \
		-Wshadow -Wformat=2 -Wformat-signedness -Wmissing-include-dirs \
		-Wstrict-overflow=2 -rdynamic -fPIC"
	)
	
	# other warnings:
	#-fstrict-aliasing  # This will not work with *reinterpret-cast<float*>(&int)
    #-Wsuggest-attribute=const|pure|etc. -Wcast-qual -Wsuggest-override # TODO
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
# Defines section
###############################################################################

add_definitions(-DBOOMERANG_VERSION="${BOOMERANG_VERSION}")
include(TestBigEndian)
#Does not work with Ubuntu 17.04 and gcc 6.3
#TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
if (WORDS_BIGENDIAN)
	add_definitions(-DBOOMERANG_BIG_ENDIAN=1)
else ()
	add_definitions(-DBOOMERANG_BIG_ENDIAN=0)
endif ()


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
    #find_package(Protobuf REQUIRED)
    
	add_subdirectory(unit_testing)
endif (BOOM_BUILD_TESTS)


# run regression suite by 'run check'
add_custom_target(check "./full_regression.sh"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)


# copy function signatures to output directory
add_custom_target(COPY_FILES ALL)
add_custom_command(TARGET COPY_FILES POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/signatures ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/signatures)

# delete all files in the 'out/' directory on make clean
set(EXTRA_CLEAN_FILES "${CMAKE_SOURCE_DIR}/out/")

SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${EXTRA_CLEAN_FILES}")

