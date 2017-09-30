#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-frontend-sources
    frontend/Decoder.h
    frontend/Frontend.cpp
    frontend/Frontend.h
    frontend/MachineInstruction.cpp
    frontend/MachineInstruction.h
    frontend/mips/mipsdecoder.cpp
    frontend/mips/mipsdecoder.h
    frontend/mips/mipsfrontend.cpp
    frontend/mips/mipsfrontend.h
    frontend/NJMCDecoder.cpp
    frontend/NJMCDecoder.h
    frontend/pentium/pentiumdecoder.cpp
    frontend/pentium/pentiumdecoder.h
    frontend/pentium/pentiumfrontend.cpp
    frontend/pentium/pentiumfrontend.h
    frontend/ppc/ppcdecoder.cpp
    frontend/ppc/ppcdecoder.h
    frontend/ppc/ppcfrontend.cpp
    frontend/ppc/ppcfrontend.h
    frontend/SigEnum.h
    frontend/sparc/sparcdecoder.cpp
    frontend/sparc/sparcdecoder.h
    frontend/sparc/sparcfrontend.cpp
    frontend/sparc/sparcfrontend.h
    frontend/st20/st20decoder.cpp
    frontend/st20/st20decoder.h
    frontend/st20/st20frontend.cpp
    frontend/st20/st20frontend.h
    frontend/TargetQueue.cpp
    frontend/TargetQueue.h
)

set(boomerang-sources "${boomerang-sources};${boomerang-frontend-sources}" PARENT_SCOPE)
