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
    main.cpp
    MiniDebugger
)

add_executable(boomerang-cli
    ${boomerang-cli-sources}
)

target_link_libraries(boomerang-cli
    boomerang
    ${CMAKE_DL_LIBS}
    Qt5::Core
)

install(TARGETS boomerang-cli
    RUNTIME DESTINATION bin/
)

