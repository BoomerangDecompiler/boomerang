#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


find_package(Qt5Core REQUIRED HINTS $ENV{QTDIR})
if (Qt5Core_FOUND)
    mark_as_advanced(Qt5Core_DIR)
endif (Qt5Core_FOUND)

find_package(Qt5Xml REQUIRED HINTS $ENV{QTDIR})
if (Qt5Xml_FOUND)
    mark_as_advanced(Qt5Xml_DIR)
endif (Qt5Xml_FOUND)

find_package(Threads)
