
macro(BOOMERANG_ADD_LOADER name)
	option(BOOM_BUILD_LOADER_${name} "Build the ${name} loader." ON)
	
	if (BOOM_BUILD_LOADER_${name})
		set(target_name "${name}Loader")
		add_library(${target_name} SHARED IFileLoader.h IBinaryFile.h ${ARGN})
		
		if (WIN32)
			set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/out/lib")
		endif (WIN32)
		qt5_use_modules(${target_name} Core)
		set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/lib")
	endif ()
endmacro()


BOOMERANG_ADD_LOADER(Elf      elf/ElfLoader.cpp elf/ElfLoader.h elf/ElfTypes.h)
BOOMERANG_ADD_LOADER(Win32    exe/pe/Win32BinaryFile.cpp exe/pe/Win32BinaryFile.h microX86dis.c)
if (MSVC AND BOOM_BUILD_LOADER_Win32)
    target_link_libraries(Win32BinaryFile Dbghelp.lib)
endif ()

BOOMERANG_ADD_LOADER(Exe      exe/dos/ExeBinaryFile.cpp exe/dos/ExeBinaryFile.h)
BOOMERANG_ADD_LOADER(DOS4GW   exe/dos4gw/DOS4GWBinaryFile.cpp exe/dos4gw/DOS4GWBinaryFile.h microX86dis.c)

BOOMERANG_ADD_LOADER(HpSom    hpsom/HpSomBinaryFile.cpp hpsom/HpSomBinaryFile.h)
BOOMERANG_ADD_LOADER(Palm     palm/PalmBinaryFile.cpp palm/PalmBinaryFile.h palm/palmsystraps.h)
BOOMERANG_ADD_LOADER(MachO    machO/MachOBinaryFile.cpp machO/MachOBinaryFile.h machO/MachOBinaryFile.cpp machO/macho-apple.h)

