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


#include "boomerang/frontend/NJMCDecoder.h"

#include <cstdlib>


class DecodeResult;
class Prog;


/**
 * The implementation of the instruction decoder for PPC.
 */
class PPCDecoder : public NJMCDecoder
{
public:
    /// \copydoc NJMCDecoder::NJMCDecoder
    PPCDecoder(Project *project);
    PPCDecoder(const PPCDecoder &other) = delete;
    PPCDecoder(PPCDecoder &&other)      = default;

    virtual ~PPCDecoder() override = default;

    PPCDecoder &operator=(const PPCDecoder &other) = delete;
    PPCDecoder &operator=(PPCDecoder &&other) = default;

public:
    /// \copydoc NJMCDecoder::decodeInstruction
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;

private:
    Exp *dis_RegImm(Address pc);

    /**
     * Decode the register
     * \param  r - register (0-31)
     * \returns  the expression representing the register
     */
    SharedExp dis_Reg(unsigned r);

    /**
     * Decode the register rA when rA represents constant 0 if r == 0
     * \param        r - register (0-31)
     * \returns             the expression representing the register
     */
    SharedExp dis_RAmbz(unsigned r); // Special for rA of certain instructions

    /**
     * Check to see if the instructions at the given offset match
     * any callee prologue, i.e. does it look like this offset
     * is a pointer to a function?
     * \param      hostPC - pointer to the code in question (host address)
     * \returns           True if a match found
     */
    bool isFuncPrologue(Address hostPC);
    DWord getDword(HostAddress lc);
};
