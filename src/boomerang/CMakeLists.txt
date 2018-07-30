#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


set(boomerang-sources "")

add_subdirectory(c)
add_subdirectory(codegen)
add_subdirectory(core)
add_subdirectory(db)
add_subdirectory(decomp)
add_subdirectory(frontend)
add_subdirectory(passes)
add_subdirectory(ssl)
add_subdirectory(type)
add_subdirectory(util)
add_subdirectory(visitor)

set(boomerang-headers "")

foreach (src ${boomerang-sources})
    string(REPLACE ".cpp" ".h" header_file ${src})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${header_file}")
        list(APPEND boomerang-headers ${header_file})
    endif ()
endforeach ()

add_library(boomerang ${boomerang-sources} ${boomerang-headers})
target_link_libraries(boomerang ${CMAKE_DL_LIBS} Qt5::Core ${DEBUG_LIB})

option(BOOMERANG_INSTALL_DEV "Install boomerang libraries and headers." OFF)

if (BOOMERANG_INSTALL_DEV)
    # install library and headers
    install(TARGETS boomerang
        ARCHIVE DESTINATION lib/boomerang
        LIBRARY DESTINATION lib/boomerang
    )

    install(DIRECTORY . DESTINATION include/boomerang
        FILES_MATCHING PATTERN "*.h"
    )
endif (BOOMERANG_INSTALL_DEV)

