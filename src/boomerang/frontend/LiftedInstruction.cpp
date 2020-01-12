#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LiftedInstruction.h"


LiftedInstructionPart::LiftedInstructionPart(std::unique_ptr<RTL> rtl)
    : m_rtl(std::move(rtl))
{
}


LiftedInstruction::LiftedInstruction()
{
}


LiftedInstruction::LiftedInstruction(LiftedInstruction &&other)
    : m_parts(std::move(other.m_parts))
{
}


LiftedInstruction::~LiftedInstruction()
{
}


LiftedInstruction &LiftedInstruction::operator=(LiftedInstruction &&other)
{
    m_parts = std::move(other.m_parts);

    return *this;
}


void LiftedInstruction::reset()
{
    m_parts.clear();
}


bool LiftedInstruction::isSimple() const
{
    return m_parts.size() == 1;
}


LiftedInstructionPart *LiftedInstruction::addPart(std::unique_ptr<RTL> rtl)
{
    m_parts.push_back(std::move(rtl));
    return &m_parts.back();
}


void LiftedInstruction::addEdge(LiftedInstructionPart *from, LiftedInstructionPart *to)
{
    assert(from != nullptr);
    assert(to != nullptr);

    from->addSuccessor(to);
    to->addPredecessor(from);
}


std::list<LiftedInstructionPart> LiftedInstruction::use()
{
    auto parts = std::move(m_parts);
    m_parts.clear();
    return parts;
}


std::unique_ptr<RTL> LiftedInstruction::useSingleRTL()
{
    assert(isSimple());
    std::unique_ptr<RTL> result = std::move(m_parts.back().m_rtl);
    m_parts.clear();
    return result;
}
