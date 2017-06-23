#pragma once

#include "boomerang/util/types.h"

#include <QString>
#include <list>
#include <memory>

class Instruction;
class RTLInstDict;
class Exp;


using SharedExp = std::shared_ptr<Exp>;


class MachineOperand
{
};

struct MachineInstruction
{
	QString                opcode;
	ADDRESS                location;
	std::vector<SharedExp> actuals;
	MachineInstruction(QString op, ADDRESS pc, std::vector<SharedExp>&& acts)
		: opcode(op)
		, location(pc)
		, actuals(acts)
	{
	}
};

class MachineSemantics
{
public:
	virtual Exp *convertOperand(MachineOperand *Operand) = 0;

	virtual std::list<Instruction *> *convertInstruction(MachineInstruction *Insn) = 0;
};
