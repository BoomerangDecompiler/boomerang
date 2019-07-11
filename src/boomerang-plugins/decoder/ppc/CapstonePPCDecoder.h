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


#include "../CapstoneDecoder.h"

#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ssl/exp/Operator.h"


/**
 * Instruction decoder using Capstone to decode
 * PPC instructions into SSL RTLs.
 */
class BOOMERANG_PLUGIN_API CapstonePPCDecoder : public CapstoneDecoder
{
public:
    CapstonePPCDecoder(Project *project);

public:
    /// \copydoc IDecoder::decodeInstruction
    bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;

    /// \copydoc IDecoder::getRegNameByNum
    QString getRegNameByNum(RegNum regNum) const override;

    /// \copydoc IDecoder::getRegSizeByNum
    int getRegSizeByNum(RegNum regNum) const override;

private:
    std::unique_ptr<RTL> createRTLForInstruction(Address pc, cs::cs_insn *instruction);

    std::unique_ptr<RTL> instantiateRTL(Address pc, const char *instructionID, int numOperands,
                                        const cs::cs_ppc_op *operands);

    /// \returns true if the instruction is a CR manipulation instruction, e.g. crxor
    bool isCRManip(const cs::cs_insn *instruction) const;
};
