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
