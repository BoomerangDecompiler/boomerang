#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

find_package(Qt5 COMPONENTS Core REQUIRED HINTS $ENV{QTDIR})
if (Qt5_FOUND)
    mark_as_advanced(Qt5_DIR Qt5Core_DIR)
endif (Qt5_FOUND)

find_package(Threads)
find_package(Capstone 4.0.1 REQUIRED)

find_package(FLEX  2.6 REQUIRED)
find_package(BISON 3.3 REQUIRED)
