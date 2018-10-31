#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

include(InstallRequiredSystemLibraries)

set(BOOMERANG_PKG_NAME "boomerang")

if (WIN32 AND NOT UNIX)
    set(BOOMERANG_PKG_PLATFORM "win")
else ()
    set(BOOMERANG_PKG_PLATFORM "linux")
endif ()

math(EXPR BOOMERANG_PKG_BITNESS "${CMAKE_SIZEOF_VOID_P} * 8")

# actual configuration
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${BOOMERANG_VERSION}")
set(CPACK_PACKAGE_VENDOR "Boomerang Developers")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Boomerang - a generic, retargetable machine code decompiler")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/Readme.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.TERMS")

set(CPACK_PACKAGE_FILE_NAME "${BOOMERANG_PKG_NAME}-${BOOMERANG_VERSION}-${BOOMERANG_PKG_PLATFORM}${BOOMERANG_PKG_BITNESS}")

if (WIN32 AND NOT UNIX)
    # There is a bug in NSI that does not handle full unix paths properly.
    # Make sure there is at least one set of four (4) backlasshes.
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${BOOMERANG_PKG_NAME}\\\\${BOOMERANG_VERSION}")
    set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}\\\\data/images/boomerang_icon.png")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Boomerang-cli.exe")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Boomerang Decompiler")
    set(CPACK_NSIS_HELP_LINK "https:\\\\\\\\github.com/BoomerangDecompiler/boomerang/issues")
    set(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\\\\\github.com/BoomerangDecompiler/boomerang/")
    set(CPACK_NSIS_MODIFY_PATH ON)

else (WIN32 AND NOT UNIX)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "")
    set(CPACK_STRIP_FILES "bin/boomerang-cli")
    set(CPACK_SOURCE_STRIP_FILES "")
endif (WIN32 AND NOT UNIX)


include(CPack)

