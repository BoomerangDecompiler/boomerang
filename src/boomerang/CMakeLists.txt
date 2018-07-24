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
target_link_libraries(boomerang ${CMAKE_DL_LIBS} Qt5::Core Capstone::Capstone ${DEBUG_LIB} capstone)


if (BUILD_SHARED_LIBS)
    target_compile_definitions(boomerang PRIVATE BOOMERANG_BUILD_SHARED=1)
else (BUILD_SHARED_LIBS) # static lib
    target_compile_definitions(boomerang PRIVATE BOOMERANG_BUILD_STATIC=1)
endif (BUILD_SHARED_LIBS)

# install library and headers
option(BOOMERANG_INSTALL_DEV "Install header files for development." OFF)

if (BOOMERANG_INSTALL_DEV OR BUILD_SHARED_LIBS)
    if (WIN32 AND NOT UNIX AND NOT BOOMERANG_INSTALL_DEV)
        # do not install boomerang.lib
        install(TARGETS boomerang
            RUNTIME DESTINATION bin
        )
    else ()
        install(TARGETS boomerang
            ARCHIVE DESTINATION lib/boomerang
            LIBRARY DESTINATION lib/boomerang
            RUNTIME DESTINATION bin
        )
    endif ()
endif ()

if (BOOMERANG_INSTALL_DEV)
    install(DIRECTORY . DESTINATION include/boomerang
        FILES_MATCHING PATTERN "*.h"
    )
endif (BOOMERANG_INSTALL_DEV)

