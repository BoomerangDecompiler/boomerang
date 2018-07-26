#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MachineInstruction.h"


#include "boomerang/db/RTL.h"
#include "boomerang/ssl/RTLInstDict.h"


class MachineSemanticsSSLBased : public MachineSemantics
{
    RTLInstDict RTLDict;

public:
    virtual ~MachineSemanticsSSLBased() {}
    Exp *convertOperand(MachineOperand *Operand) override;

    std::unique_ptr<RTL> convertInstruction(MachineInstruction *Insn) override;
};


Exp *MachineSemanticsSSLBased::convertOperand(MachineOperand *Operand)
{
    Q_UNUSED(Operand);
    return nullptr;
}


std::unique_ptr<RTL> MachineSemanticsSSLBased::convertInstruction(MachineInstruction *Insn)
{
    return RTLDict.instantiateRTL(Insn->opcode, Insn->location, Insn->actuals);
}
