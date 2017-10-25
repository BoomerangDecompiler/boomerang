#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/signatures" DESTINATION "lib/boomerang")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/ssl"   DESTINATION "lib/boomerang")

# installation
install(FILES ${SSL_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/boomerang/frontend/")


