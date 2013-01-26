INCLUDE_DIRECTORIES(../c/) # used by FrontEnd::readLibrarySignatures
SET(SRC
    frontend.cpp #-Ic
    njmcDecoder.cpp
    pentiumdecoder.cpp #-fno-exceptions
    pentiumfrontend.cpp
    ppcdecoder.cpp
    ppcfrontend.cpp
    sparcdecoder.cpp
    sparcfrontend.cpp
    mipsdecoder.cpp
    mipsfrontend.cpp
    st20decoder.cpp
    st20frontend.cpp
)
ADD_LIBRARY(frontend STATIC ${SRC})
