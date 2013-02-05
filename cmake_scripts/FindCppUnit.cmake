#
# Find the CppUnit includes and library
#
# This module defines
# CPPUNIT_INCLUDE_DIR, where to find tiff.h, etc.
# CPPUNIT_LIBRARIES, the libraries to link against to use CppUnit.
# CPPUNIT_FOUND, If false, do not try to use CppUnit.

# also defined, but not for general use are
# CPPUNIT_LIBRARY, where to find the CppUnit library.
# CPPUNIT_DEBUG_LIBRARY, where to find the CppUnit library in debug mode.
IF(CPPUNIT_LIBRARIES AND CPPUNIT_INCLUDE_DIR)
	# in cache already
	SET(CPPUNIT_FOUND TRUE)
ELSE(CPPUNIT_LIBRARIES AND CPPUNIT_INCLUDE_DIR)
	FIND_PATH(CPPUNIT_INCLUDE_DIR cppunit/TestCase.h
	/usr/local/include
	/usr/include
	)

	# With Win32, important to have both
	IF(WIN32)
		FIND_LIBRARY(CPPUNIT_LIBRARY 
			NAMES 
				cppunit
			PATHS
				${CPPUNIT_INCLUDE_DIR}/../lib
				/usr/local/lib
				/usr/lib
		)
		FIND_LIBRARY(CPPUNIT_DEBUG_LIBRARY 
			NAMES
				cppunitd
			PATHS
				${CPPUNIT_INCLUDE_DIR}/../lib
					/usr/local/lib
					/usr/lib
		)
	ELSE(WIN32)
		FIND_LIBRARY(CPPUNIT_LIBRARY 
			NAMES
				cppunit
			PATHS
				${CPPUNIT_INCLUDE_DIR}/../lib
				/usr/local/lib
				/usr/lib
		)
		# On unix system, debug and release have the same name
		FIND_LIBRARY(CPPUNIT_DEBUG_LIBRARY 
			NAMES
				cppunit
			PATHS
				${CPPUNIT_INCLUDE_DIR}/../lib
				/usr/local/lib
				/usr/lib
		)
	ENDIF(WIN32)

	IF(CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)
			SET(CPPUNIT_FOUND TRUE)
			SET(CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARY} ${CMAKE_DL_LIBS})
			SET(CPPUNIT_DEBUG_LIBRARIES ${CPPUNIT_DEBUG_LIBRARY}
		${CMAKE_DL_LIBS})
	ENDIF(CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)

	IF(CPPUNIT_FOUND)
		IF(NOT CppUnit_FIND_QUIETLY)
			MESSAGE(STATUS "Found CppUnit: ${CPPUNIT_LIBRARIES}")
		ENDIF (NOT CppUnit_FIND_QUIETLY)
	ELSE (CPPUNIT_FOUND)
		IF (CppUnit_FIND_REQUIRED)
			MESSAGE(FATAL_ERROR "Could not find CppUnit")
		ENDIF (CppUnit_FIND_REQUIRED)
	ENDIF (CPPUNIT_FOUND)
	# show the CPPUNIT_INCLUDE_DIR and CPPUNIT_LIBRARIES variables only in the advanced view
	MARK_AS_ADVANCED(CPPUNIT_INCLUDE_DIR CPPUNIT_LIBRARIES)
ENDIF(CPPUNIT_LIBRARIES AND CPPUNIT_INCLUDE_DIR)
