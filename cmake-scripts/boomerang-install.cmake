#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# Make sure libbooomerang is found
if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif(APPLE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/boomerang")


# always install those
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/signatures/" DESTINATION "share/boomerang/signatures")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/ssl/"        DESTINATION "share/boomerang/ssl")

if (BOOMERANG_BUILD_GUI)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/images/" DESTINATION "share/boomerang/images")
endif (BOOMERANG_BUILD_GUI)

if (BOOMERANG_INSTALL_SAMPLES)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/samples/" DESTINATION "share/boomerang/samples")
endif (BOOMERANG_INSTALL_SAMPLES)
