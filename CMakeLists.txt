project(Boomerang)

###############################################################################
# Settings related to build system, and target directories
###############################################################################
cmake_minimum_required(VERSION 3.1)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_scripts)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_SHARED_MODULE_PREFIX ) # prevent windows/mingw modules having lib* prefix

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
if (WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/out)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/out)
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif ()

###############################################################################
# Options section
###############################################################################
option(BUILD_TESTING "Build the testing tree." ON)

# help the cmake find 3rd_party compiled/installed packages
set(CMAKE_PREFIX_PATH  "${CMAKE_PREFIX_PATH};${PROJECT_SOURCE_DIR}/3rd_party")

IF (BUILD_TESTING)
    enable_testing()
    find_package(Qt5Test)
    #find_package(Protobuf REQUIRED)
ENDIF (BUILD_TESTING)

###############################################################################
# Compiler specific flags section
###############################################################################

set(COMMON_COMPILE_OPTIONS "")

if (CMAKE_CXX_COMPILER MATCHES cl.exe) # MSVC
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
elseif (CMAKE_CXX_COMPILER MATCHES clang)
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything  -Wno-padded -Wno-c++98-compat -Wno-weak-vtables -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-exit-time-destructors" )
	#add_definitions(-D_GLIBCXX_DEBUG) #-D_GLIBCXX_DEBUG
else ()
	set(COMMON_COMPILE_OPTIONS "-Wall -Wextra -Werror -Werror=pedantic -Wfatal-errors \
		-Wshadow -Wformat=2 -Wformat-signedness -Wmissing-include-dirs -Wstrict-overflow=2")
		
	# other warnings:
	#-fstrict-aliasing  # This will not work with *reinterpret-cast<float*>(&int)
    #-Wsuggest-attribute=const|pure|etc. -Wcast-qual -Wsuggest-override # TODO
	#-Wundef -Wzero-as-null-pointer-constant #does not compile with boost
	
	# Enable additional warnings for GCC6+
	if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
		set(COMMON_COMPILE_OPTIONS "${COMMON_COMPILE_PTIONS} \
			-Wnull-dereference -Wduplicated-cond")
			#-Walloc-zero -Walloca -Wduplicated-branches 
	endif ()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COMPILE_OPTIONS} -std=c++11")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${COMMON_COMPILE_OPTIONS}")
    
    add_definitions(-D__STDC_FORMAT_MACROS) #-D_GLIBCXX_DEBUG
    #add_definitions(-D_GLIBCXX_DEBUG) #-D_GLIBCXX_DEBUG
endif ()

if (MSVC)
    set(DEBUG_LIB Dbghelp.lib)
endif ()

include(TestBigEndian)

# Find Qt
find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Threads)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#Does not work with Ubuntu 17.04 and gcc 6.3
#TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

###############################################################################
# Defines section
###############################################################################
set(VERSION "alpha 0.3.99 xx/xxx/2016")
set(MAIN_INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
set(PACKAGE_BUGREPORT "http://github.com/nemerle/boomerang")
set(PACKAGE_NAME      "boomerang")
set(PACKAGE_STRING    "boomerang")
set(PACKAGE_TARNAME   "boomerang")
set(PACKAGE_VERSION   "0.3.99")

#if git is installed (likely), pull version string from git
find_package(Git)
if (GIT_FOUND)
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" describe --dirty --tags
		RESULT_VARIABLE DESCRIBE_FAILED
		OUTPUT_VARIABLE VERSION_TEMP
	)

	if (NOT DESCRIBE_FAILED)
		set(VERSION VERSION_TEMP)
		set(PACKAGE_VERSION VERSION_TEMP)
	endif (NOT DESCRIBE_FAILED)
endif (GIT_FOUND)


# generate config.h
configure_file(include/config.h.cmake include/config.h)

include_directories(
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/3rd_party"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    ${Qt5Core_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
)

link_directories(
    "${PROJECT_SOURCE_DIR}/3rd_party/lib"
    "${CMAKE_CURRENT_BINARY_DIR}/loader"
)

add_subdirectory(src)

add_subdirectory(frontend) # TODO

if (BUILD_TESTING)
	add_subdirectory(unit_testing)
endif (BUILD_TESTING)



add_subdirectory(loader)
add_subdirectory(ui)

# run regression suite by 'run check'
add_custom_target(check "./full_regression.sh"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)


# copy function signatures to output directory
add_custom_target(COPY_FILES ALL)
add_custom_command(TARGET COPY_FILES POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/signatures ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/signatures)

