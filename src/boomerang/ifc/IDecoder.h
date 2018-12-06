#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/frontend/DecodeResult.h"


class Exp;
class RTL;
class Prog;
class RTLInstDict;


/**
 * Base class for machine instruction decoders.
 * Decoders translate raw bytes to statement lists (RTLs).
 */
class IDecoder
{
public:
    virtual ~IDecoder() = default;

    /**
     * Decodes the machine instruction at \p pc.
     * The decode result is stored into \p result, if the decode was successful.
     * If the decode was not successful, the content of \p result is undefined.
     * \returns true iff decoding the instruction was successful.
     */
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) = 0;

    /// \returns machine-specific register name given its index
    virtual QString getRegName(int regID) const = 0;

    /// \returns index of the named register
    virtual int getRegIdx(const QString &name) const = 0;

    /// \returns size of register in bits
    virtual int getRegSize(int regID) const = 0;

    /// \returns the size of the register with name \p name, in bits
    int getRegSize(const QString &name) const { return getRegSize(getRegIdx(name)); }

    virtual const RTLInstDict *getDict() const = 0;
};
