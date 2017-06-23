#include "MachineInstruction.h"

#include "boomerang/db/rtl.h"

class MachineSemanticsSSLBased : public MachineSemantics
{
	RTLInstDict RTLDict;

public:
	Exp *convertOperand(MachineOperand *Operand) override;

	std::list<Instruction *> *convertInstruction(MachineInstruction *Insn) override;
};

Exp *MachineSemanticsSSLBased::convertOperand(MachineOperand *Operand)
{
	Q_UNUSED(Operand);
	return nullptr;
}


std::list<Instruction *> *MachineSemanticsSSLBased::convertInstruction(MachineInstruction *Insn)
{
	return RTLDict.instantiateRTL(Insn->opcode, Insn->location, Insn->actuals);
}
