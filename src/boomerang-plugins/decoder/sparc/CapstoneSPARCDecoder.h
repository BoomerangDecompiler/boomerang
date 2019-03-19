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


class BOOMERANG_API CapstoneSPARCDecoder : public CapstoneDecoder
{
public:
    CapstoneSPARCDecoder(Project *project);

public:
    /// \copydoc IDecoder::decodeInstruction
    bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;

    /// \copydoc IDecoder::getRegNameByNum
    QString getRegNameByNum(RegNum regNum) const override;

    /// \copydoc IDecoder::getRegNumByName
    RegNum getRegNumByName(const QString &name) const override;

    /// \copydoc IDecoder::getRegSizeByNum
    int getRegSizeByNum(RegNum regNum) const override;

private:
    std::unique_ptr<RTL> createRTLForInstruction(Address pc, cs::cs_insn *instruction);

    std::unique_ptr<RTL> instantiateRTL(Address pc, const char *instructionID, int numOperands,
                                        const cs::cs_sparc_op *operands);

    ICLASS getInstructionType(const cs::cs_insn *instruction);
};
