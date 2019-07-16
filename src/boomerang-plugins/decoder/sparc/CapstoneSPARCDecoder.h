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


/**
 * Instruction decoder using Capstone to decode
 * SPARC instructions into SSL RTLs.
 */
class BOOMERANG_PLUGIN_API CapstoneSPARCDecoder : public CapstoneDecoder
{
public:
    CapstoneSPARCDecoder(Project *project);

public:
    /// \copydoc IDecoder::decodeInstruction
    bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;

    /// \copydoc IDecoder::getRegNameByNum
    QString getRegNameByNum(RegNum regNum) const override;

    /// \copydoc IDecoder::getRegSizeByNum
    int getRegSizeByNum(RegNum regNum) const override;

    /// \copydoc IDecoder::isSPARCRestore
    bool isSPARCRestore(Address pc, ptrdiff_t delta) const override;

private:
    std::unique_ptr<RTL> createRTLForInstruction(Address pc, cs::cs_insn *instruction);

    std::unique_ptr<RTL> instantiateRTL(Address pc, const char *instructionID,
                                        const cs::cs_insn *instruction);

    /// \returns the delay slot behaviour type of an instruction.
    IClass getInstructionType(const cs::cs_insn *instruction);

    /// Translate Capstone register ID to Boomerang internal register ID.
    RegNum fixRegNum(int csRegID) const;

    /// Translate the Capstone register ID of an instruction operand to the corresponding Boomerang
    /// internal register ID.
    /// \note This function can only be called for CS_OP_REG operands.
    RegNum fixRegNum(const cs::cs_insn *insn, int opIdx) const;

    /// \returns the regOf expression corresponding to the Capstone register ID
    /// (e.g. cs::SPARC_REG_L0 -> r16)
    SharedExp getRegExp(int csRegID) const;

    /// \returns the regOf expression corresponding to the instruction operand with index \p opIdx.
    /// \note This function can only be called for CS_OP_REG operands.
    /// \sa operandToExp
    SharedExp getRegExp(const cs::cs_insn *instruction, int opIdx) const;

    /// \returns the expression of the instruction operand with index \p opIdx
    SharedExp operandToExp(const cs::cs_insn *instruction, int opIdx) const;

    /// For register operands, returns the size of the register, in bits.
    /// This is because some instructions only specify the first register of a double register
    /// operation.
    /// Example: fsqrtd %f2, %f4 reads %f2 and %f3, and writes to %f4 and %f5
    int getRegOperandSize(const cs::cs_insn *instruction, int opIdx) const;

    /// Decode LDD instruction manually. Can be removed when upgrading to Capstone 5.
    bool decodeLDD(cs::cs_insn *instruction, uint32_t instructionData) const;

    /// Decode STD instruction manually. Can be removed when upgrading to Capstone 5.
    bool decodeSTD(cs::cs_insn *instruction, uint32_t instructionData) const;
};
