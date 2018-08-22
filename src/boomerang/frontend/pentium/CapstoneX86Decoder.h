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
    /**
     * Creates a new RTL for a single instruction.
     * \param pc the address of the instruction to instantiate.
     * \param instruction the actual instruction.
     *
     * \internal Note that for some instruction groups (e.g. calls, jumps, setCC instructions)
     * hard-coded adjustments are performed due to SSL limitations. See the function definition
     * for details.
     */
    std::unique_ptr<RTL> createRTLForInstruction(Address pc, const cs::cs_insn *instruction);

    /**
     * Instantiates an RTL for a single instruction, replacing formal parameters with actual
     * arguments from \p operands.
     *
     * \param pc the address of the instruction.
     * \param instructionID the unique name of the instruction (e.g. MOV.reg32.reg32)
     * \param numOperands number of instruction operands (e.g. 2 for MOV.reg32.reg32)
     * \param operands Array containing actual arguments containing \p numOperands elements.
     */
    std::unique_ptr<RTL> instantiateRTL(Address pc, const char *instructionID, int numOperands,
                                        const cs::cs_x86_op *operands);
};
