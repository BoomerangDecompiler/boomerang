#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


include(boomerang-utils)

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
)

set(IFC_SOURCES
    ${CMAKE_SOURCE_DIR}/src/boomerang/ifc/IFileLoader.h
)

BOOMERANG_ADD_LOADER(
    NAME Elf
    SOURCES ${IFC_SOURCES} elf/ElfBinaryLoader.cpp elf/ElfBinaryLoader.h elf/ElfTypes.h
)

BOOMERANG_ADD_LOADER(
    NAME Exe
    SOURCES ${IFC_SOURCES} exe/ExeBinaryLoader.cpp exe/ExeBinaryLoader.h
)

BOOMERANG_ADD_LOADER(
    NAME DOS4GW
    SOURCES ${IFC_SOURCES} dos4gw/DOS4GWBinaryLoader.cpp dos4gw/DOS4GWBinaryLoader.h microX86dis.c
)

BOOMERANG_ADD_LOADER(
    NAME MachO
    SOURCES ${IFC_SOURCES} machO/MachOBinaryLoader.cpp machO/MachOBinaryLoader.h machO/MachOBinaryLoader.cpp machO/macho-apple.h
)

BOOMERANG_ADD_LOADER(
    NAME ST20
    SOURCES ${IFC_SOURCES} st20/ST20BinaryLoader.cpp st20/ST20BinaryLoader.h
)

if (MSVC)
    BOOMERANG_ADD_LOADER(
        NAME Win32
        SOURCES ${IFC_SOURCES} pe/Win32BinaryLoader.cpp pe/Win32BinaryLoader.h microX86dis.c
        LIBRARIES Dbghelp.lib
    )
else ()
    BOOMERANG_ADD_LOADER(
        NAME Win32
        SOURCES ${IFC_SOURCES} pe/Win32BinaryLoader.cpp pe/Win32BinaryLoader.h microX86dis.c
    )
endif (MSVC)

