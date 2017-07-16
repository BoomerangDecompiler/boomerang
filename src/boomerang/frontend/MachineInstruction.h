#pragma once

#include "boomerang/util/Address.h"

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
    Address                location;
    std::vector<SharedExp> actuals;

    MachineInstruction(QString op, Address pc, std::vector<SharedExp>&& acts)
        : opcode(op)
        , location(pc)
        , actuals(acts)
    {
    }
};

class MachineSemantics
{
public:
    virtual ~MachineSemantics() {}

    virtual Exp *convertOperand(MachineOperand *Operand) = 0;

    virtual std::list<Instruction *> *convertInstruction(MachineInstruction *Insn) = 0;
};
