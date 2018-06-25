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


#include "boomerang/ifc/IDecoder.h"
#include "boomerang/ssl/RTLInstDict.h"


namespace cs
{
#include <capstone/capstone.h>
}


class CapstoneDecoder : public IDecoder
{
public:
    CapstoneDecoder(Prog *prog);
    virtual ~CapstoneDecoder();

public:
    /**
     * Decodes the machine instruction at \p pc.
     * The decode result is stored into \p result, if the decode was successful.
     * If the decode was not successful, the content of \p result is undefined.
     * \returns true if decoding the instruction was successful.
     */
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;

    /// \returns machine-specific register name given it's index
    virtual QString getRegName(int idx) const override;

    /// \returns index of the named register
    virtual int getRegIdx(const QString &name) const override;

    /// \returns size of register in bits
    virtual int getRegSize(int idx) const override;

private:
    ICLASS getInstructionClass(const cs::cs_insn *instruction);
    std::unique_ptr<RTL> getRTL(Address pc, const cs::cs_insn *instruction);
    std::unique_ptr<RTL> instantiateRTL(Address pc, const char *instructionID, int numOperands,
                                        const cs::cs_x86_op *operands);

    bool isInstructionInGroup(const cs::cs_insn *instruction, uint8_t group);

private:
    cs::csh m_handle;
    Prog *m_prog;
    RTLInstDict m_dict;
};
