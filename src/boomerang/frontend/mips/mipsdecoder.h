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

#include <cstddef>

class Prog;
struct DecodeResult;

/**
 * Decoder for the MIPS instruction set.
 * \author Markus Gothe, nietzsche@lysator.liu.se
 */
class MIPSDecoder : public NJMCDecoder
{
public:
    /// \copydoc NJMCDecoder::NJMCDecoder
    MIPSDecoder(Prog *prog);

    /// \copydoc NJMCDecoder::decodeInstruction
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult& result) override;
};
