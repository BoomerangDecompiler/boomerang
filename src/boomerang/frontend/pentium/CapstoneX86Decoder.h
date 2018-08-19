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


#include "boomerang/frontend/CapstoneDecoder.h"


/**
 * Instruction decoder using capstone to decode
 * x86_32 instructions into SSL RTLs.
 */
class CapstoneX86Decoder : public CapstoneDecoder
{
public:
    CapstoneX86Decoder(Prog *prog);

public:
    /// \copydoc IDecoder::decodeInstruction
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;

    /// \copydoc IDecoder::getRegName
    virtual QString getRegName(int regID) const override;

    /// \copydoc IDecoder::getRegIdx
    virtual int getRegIdx(const QString &name) const override;

    /// \copydoc IDecoder::getRegSize
    virtual int getRegSize(int regID) const override;

private:
    ICLASS getInstructionClass(const cs::cs_insn *instruction);
    std::unique_ptr<RTL> getRTL(Address pc, const cs::cs_insn *instruction);
    std::unique_ptr<RTL> instantiateRTL(Address pc, const char *instructionID, int numOperands,
                                        const cs::cs_x86_op *operands);
};
