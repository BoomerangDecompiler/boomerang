#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-frontend-sources
    frontend/DecodeResult
    frontend/Frontend
    frontend/MachineInstruction
    frontend/mips/mipsdecoder
    frontend/mips/mipsfrontend
    frontend/NJMCDecoder
    frontend/pentium/pentiumdecoder
    frontend/pentium/pentiumfrontend
    frontend/pentium/StringInstructionProcessor
    frontend/ppc/ppcdecoder
    frontend/ppc/ppcfrontend
    frontend/SigEnum
    frontend/sparc/sparcdecoder
    frontend/sparc/sparcfrontend
    frontend/st20/st20decoder
    frontend/st20/st20frontend
    frontend/TargetQueue
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-frontend-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-frontend-sources}" PARENT_SCOPE)
