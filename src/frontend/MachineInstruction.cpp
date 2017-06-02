#include "MachineInstruction.h"

#include "db/rtl.h"

class MachineSemanticsSSLBased : public MachineSemantics
{
	RTLInstDict RTLDict;

	// MachineSemantics interface

public:
	Exp *convertOperand(MachineOperand *Operand);

	std::list<Instruction *> *convertInstruction(MachineInstruction *Insn);
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
