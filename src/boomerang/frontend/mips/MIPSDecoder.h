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
class DecodeResult;


/**
 * Decoder for the MIPS instruction set.
 * \author Markus Gothe, nietzsche@lysator.liu.se
 */
class MIPSDecoder : public NJMCDecoder
{
public:
    /// \copydoc NJMCDecoder::NJMCDecoder
    MIPSDecoder(Prog *prog);
    MIPSDecoder(const MIPSDecoder &other) = delete;
    MIPSDecoder(MIPSDecoder &&other)      = default;

    virtual ~MIPSDecoder() override = default;

    MIPSDecoder &operator=(const MIPSDecoder &other) = delete;
    MIPSDecoder &operator=(MIPSDecoder &&other) = default;

public:
    /// \copydoc NJMCDecoder::decodeInstruction
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;
};
