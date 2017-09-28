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
    ${CMAKE_SOURCE_DIR}/src/boomerang/loader/IFileLoader.h
    ${CMAKE_SOURCE_DIR}/src/boomerang/loader/IBinaryFile.h
)

BOOMERANG_ADD_LOADER(
    NAME Elf
    SOURCES ${IFC_SOURCES} elf/ElfBinaryLoader.cpp elf/ElfBinaryLoader.h elf/ElfTypes.h
)

BOOMERANG_ADD_LOADER(
    NAME Exe
    SOURCES ${IFC_SOURCES} exe/dos/ExeBinaryLoader.cpp exe/dos/ExeBinaryLoader.h
)

BOOMERANG_ADD_LOADER(
    NAME DOS4GW
    SOURCES ${IFC_SOURCES} exe/dos4gw/DOS4GWBinaryLoader.cpp exe/dos4gw/DOS4GWBinaryLoader.h microX86dis.c
)

BOOMERANG_ADD_LOADER(
    NAME HpSom
    SOURCES ${IFC_SOURCES} hpsom/HpSomBinaryLoader.cpp hpsom/HpSomBinaryLoader.h
)

BOOMERANG_ADD_LOADER(
    NAME Palm
    SOURCES ${IFC_SOURCES} palm/PalmBinaryLoader.cpp palm/PalmBinaryLoader.h palm/palmsystraps.h
)

BOOMERANG_ADD_LOADER(
    NAME MachO
    SOURCES ${IFC_SOURCES} machO/MachOBinaryLoader.cpp machO/MachOBinaryLoader.h machO/MachOBinaryLoader.cpp machO/macho-apple.h
)

if (MSVC)
    BOOMERANG_ADD_LOADER(
        NAME Win32
        SOURCES ${IFC_SOURCES} exe/pe/Win32BinaryLoader.cpp exe/pe/Win32BinaryLoader.h microX86dis.c
        LIBRARIES Dbghelp.lib
    )
else ()
    BOOMERANG_ADD_LOADER(
        NAME Win32
        SOURCES ${IFC_SOURCES} exe/pe/Win32BinaryLoader.cpp exe/pe/Win32BinaryLoader.h microX86dis.c
    )
endif (MSVC)

