INCLUDE_DIRECTORIES(../c/) # used by FrontEnd::readLibrarySignatures
SET(SRC
    frontend.cpp #-Ic
    TargetQueue.cpp #-Ic
    njmcDecoder.cpp
    pentium/pentiumdecoder.cpp #-fno-exceptions
    pentium/pentiumfrontend.cpp
    ppc/ppcdecoder.cpp
    ppc/ppcfrontend.cpp
    sparc/sparcdecoder.cpp
    sparc/sparcdecoder.h
    sparc/sparcfrontend.cpp
    sparc/sparcfrontend.h
#    mc68k/m68kfrontend.cpp
#    mc68k/m68kdecoder.cpp
    mips/mipsdecoder.cpp
    mips/mipsdecoder.h
    mips/mipsfrontend.cpp
    mips/mipsfrontend.h
    st20/st20decoder.cpp
    st20/st20decoder.h
    st20/st20frontend.cpp
    st20/st20frontend.h
)
ADD_LIBRARY(frontend STATIC ${SRC})
qt5_use_modules(frontend Core)

IF(BUILD_TESTING)
#ADD_SUBDIRECTORY(unit_testing)
ENDIF()
