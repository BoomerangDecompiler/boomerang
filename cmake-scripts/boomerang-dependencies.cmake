#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


# Find latest qt version
set(POSSIBLE_QT_LOCATIONS C:/Qt)
if (DEFINED ENV{QTDIR})
    set(QTDIR_TEMP $ENV{QTDIR})
    string(REPLACE "\\" "/" QTDIR_TEMP "${QTDIR_TEMP}")
    list(APPEND POSSIBLE_QT_LOCATIONS "${QTDIR_TEMP}")
endif (DEFINED ENV{QTDIR})

foreach (POSSIBLE_QT_LOCATION ${POSSIBLE_QT_LOCATIONS})
    if (EXISTS "${POSSIBLE_QT_LOCATION}")
        # found qt
        foreach (_comp Core Test Xml Widgets)
            set(COMP_DIR "${POSSIBLE_QT_LOCATION}/lib/cmake/Qt5${_comp}/")
            if (EXISTS "${COMP_DIR}")
                set(Qt5${_comp}_DIR "${COMP_DIR}" CACHE STRING "")
                mark_as_advanced(Qt5${_comp}_DIR)
                list(APPEND CMAKE_PREFIX_PATH "${COMP_DIR}")
            endif ()
        endforeach ()
    endif (EXISTS "${POSSIBLE_QT_LOCATION}")
endforeach ()


find_package(Qt5Core REQUIRED HINTS $ENV{QTDIR})
if (Qt5Core_FOUND)
    mark_as_advanced(Qt5Core_DIR)
endif (Qt5Core_FOUND)

find_package(Qt5Xml REQUIRED HINTS $ENV{QTDIR})
if (Qt5Xml_FOUND)
    mark_as_advanced(Qt5Xml_DIR)
endif (Qt5Xml_FOUND)

find_package(Threads)
find_package(Capstone REQUIRED)
