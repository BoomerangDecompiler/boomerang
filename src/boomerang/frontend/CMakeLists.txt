#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-frontend-sources
    frontend/DecodeResult
    frontend/DefaultFrontEnd
    frontend/mips/MIPSDecoder
    frontend/mips/MIPSFrontEnd
    frontend/NJMCDecoder
    frontend/pentium/PentiumDecoder
    frontend/pentium/PentiumFrontEnd
    frontend/pentium/StringInstructionProcessor
    frontend/ppc/PPCDecoder
    frontend/ppc/PPCFrontEnd
    frontend/SigEnum
    frontend/sparc/SPARCDecoder
    frontend/sparc/SPARCFrontEnd
    frontend/st20/ST20Decoder
    frontend/st20/ST20FrontEnd
    frontend/TargetQueue
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-frontend-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-frontend-sources}" PARENT_SCOPE)
