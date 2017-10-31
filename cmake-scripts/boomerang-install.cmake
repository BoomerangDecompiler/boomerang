#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


# always install those
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/signatures/" DESTINATION "share/boomerang/signatures")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/ssl/" DESTINATION "share/boomerang/ssl")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/transformations/" DESTINATION "share/boomerang/transformations")


if (BOOMERANG_BUILD_GUI)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/images/" DESTINATION "share/boomerang/images")
endif (BOOMERANG_BUILD_GUI)

if (BOOMERANG_INSTALL_SAMPLES)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/samples/" DESTINATION "share/boomerang/samples")
endif (BOOMERANG_INSTALL_SAMPLES)
