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
find_package(Capstone 3.0 REQUIRED)

find_package(FLEX  2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

if (BISON_VERSION VERSION_LESS 3.0.5)
    message(WARNING "It is recommended to use Bison 3.0.5 or later")
endif ()
