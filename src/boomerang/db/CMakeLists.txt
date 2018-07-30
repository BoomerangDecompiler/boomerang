#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


list(APPEND boomerang-db-sources
    db/BasicBlock
    db/CFG
    db/DataFlow
    db/DebugInfo
    db/DefCollector
    db/Global
    db/Module
    db/Prog
    db/UseCollector

    db/binary/BinaryFile
    db/binary/BinaryImage
    db/binary/BinarySection
    db/binary/BinarySymbol
    db/binary/BinarySymbolTable

    db/proc/LibProc
    db/proc/Proc
    db/proc/UserProc

    db/signature/CustomSignature
    db/signature/MIPSSignature
    db/signature/Signature
    db/signature/Parameter
    db/signature/Return
    db/signature/PentiumSignature
    db/signature/PPCSignature
    db/signature/SparcSignature
    db/signature/ST20Signature
    db/signature/Win32Signature
)

BOOMERANG_LIST_APPEND_FOREACH(boomerang-db-sources ".cpp")

set(boomerang-sources "${boomerang-sources};${boomerang-db-sources}" PARENT_SCOPE)
