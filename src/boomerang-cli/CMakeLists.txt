#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


set(CMAKE_AUTOMOC ON)


set(boomerang-cli-sources
    Console
    CommandlineDriver
    Main
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-cli-sources ".cpp")
set(boomerang-cli-headers "")

foreach (src ${boomerang-cli-sources})
    string(REPLACE ".cpp" ".h" header_file ${src})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${header_file}")
        list(APPEND boomerang-cli-headers ${header_file})
    endif ()
endforeach ()

add_executable(boomerang-cli
    ${boomerang-cli-sources}
    ${boomerang-cli-headers}
)

target_link_libraries(boomerang-cli
    boomerang
    ${CMAKE_DL_LIBS}
    Qt5::Core
)

install(TARGETS boomerang-cli
    RUNTIME DESTINATION bin/
)

if (WIN32 AND NOT UNIX)
    include(boomerang-windeployqt)
    BOOMERANG_WINDEPLOYQT(boomerang-cli bin/)
endif ()
