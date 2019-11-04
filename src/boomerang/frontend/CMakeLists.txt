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
    frontend/MachineInstruction
    frontend/SigEnum
    frontend/TargetQueue
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-frontend-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-frontend-sources}" PARENT_SCOPE)
