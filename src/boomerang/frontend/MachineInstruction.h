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


#include "boomerang/util/Address.h"

#include <QString>
#include <list>
#include <memory>

class Statement;
class RTLInstDict;
class Exp;


using SharedExp = std::shared_ptr<Exp>;


class MachineOperand
{
};


struct MachineInstruction
{
public:
    MachineInstruction(QString op, Address pc, std::vector<SharedExp>&& acts)
        : opcode(op)
        , location(pc)
        , actuals(acts)
    {
    }

public:
    QString                opcode;
    Address                location;
    std::vector<SharedExp> actuals;

};


class MachineSemantics
{
public:
    virtual ~MachineSemantics() = default;

    virtual Exp *convertOperand(MachineOperand *Operand) = 0;

    virtual std::list<Statement *> *convertInstruction(MachineInstruction *Insn) = 0;
};
