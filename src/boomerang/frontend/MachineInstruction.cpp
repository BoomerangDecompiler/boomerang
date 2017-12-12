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
#include "boomerang/db/ssl/RTLInstDict.h"


class MachineSemanticsSSLBased : public MachineSemantics
{
    RTLInstDict RTLDict;

public:
    virtual ~MachineSemanticsSSLBased() {}
    Exp *convertOperand(MachineOperand *Operand) override;

    std::list<Statement *> *convertInstruction(MachineInstruction *Insn) override;
};


Exp *MachineSemanticsSSLBased::convertOperand(MachineOperand *Operand)
{
    Q_UNUSED(Operand);
    return nullptr;
}


std::list<Statement *> *MachineSemanticsSSLBased::convertInstruction(MachineInstruction *Insn)
{
    return RTLDict.instantiateRTL(Insn->opcode, Insn->location, Insn->actuals);
}
