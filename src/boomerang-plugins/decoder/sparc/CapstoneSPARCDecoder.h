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

    /// \copydoc IDecoder::isSPARCRestore
    bool isSPARCRestore(Address pc, ptrdiff_t delta) const override;

private:
    std::unique_ptr<RTL> createRTLForInstruction(Address pc, cs::cs_insn *instruction);

    std::unique_ptr<RTL> instantiateRTL(Address pc, const char *instructionID, const cs::cs_insn *instruction);

    ICLASS getInstructionType(const cs::cs_insn *instruction);

    RegNum fixRegNum(int csRegID) const;
    RegNum fixRegNum(const cs::cs_insn *insn, int opIdx) const;
    SharedExp getRegExp(int csRegID) const;
    SharedExp getRegExp(const cs::cs_insn *instruction, int opIdx) const;
    SharedExp operandToExp(const cs::cs_insn *instruction, int opIdx) const;

    /// for register operands, returns the size of the register, in bits
    int getRegOperandSize(const cs::cs_insn *instruction, int opIdx) const;

    bool decodeLDD(cs::cs_insn *instruction, uint32_t instructionData) const;
    bool decodeSTD(cs::cs_insn *instruction, uint32_t instructionData) const;
};
