#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-frontend-sources
    frontend/Decoder.h
    frontend/Frontend
    frontend/MachineInstruction
    frontend/mips/mipsdecoder
    frontend/mips/mipsfrontend
    frontend/NJMCDecoder
    frontend/pentium/pentiumdecoder
    frontend/pentium/pentiumfrontend
    frontend/ppc/ppcdecoder
    frontend/ppc/ppcfrontend
    frontend/SigEnum.h
    frontend/sparc/sparcdecoder
    frontend/sparc/sparcfrontend
    frontend/st20/st20decoder
    frontend/st20/st20frontend
    frontend/TargetQueue
)

set(boomerang-sources "${boomerang-sources};${boomerang-frontend-sources}" PARENT_SCOPE)
