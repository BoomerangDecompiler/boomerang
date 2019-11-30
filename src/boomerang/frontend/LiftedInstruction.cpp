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


LiftedInstruction::LiftedInstruction()
{
    reset();
}


LiftedInstruction::LiftedInstruction(LiftedInstruction &&other)
    : m_rtls(std::move(other.m_rtls))
{
}


LiftedInstruction::~LiftedInstruction()
{
}


LiftedInstruction &LiftedInstruction::operator=(LiftedInstruction &&other)
{
    m_rtls = std::move(other.m_rtls);

    return *this;
}


void LiftedInstruction::reset()
{
    m_rtls.clear();
}


void LiftedInstruction::appendRTL(std::unique_ptr<RTL> rtl, int numRTLsBefore)
{
    assert(m_rtls.size() == (std::size_t)numRTLsBefore);
    Q_UNUSED(numRTLsBefore);

    m_rtls.push_back(std::move(rtl));
}


std::unique_ptr<RTL> LiftedInstruction::useSingleRTL()
{
    assert(this->isSingleRTL());
    std::unique_ptr<RTL> rtl = std::move(m_rtls.front());
    reset();
    return rtl;
}


RTLList LiftedInstruction::useRTLs()
{
    RTLList &&rtls = std::move(m_rtls);
    reset();
    return std::move(rtls);
}


void LiftedInstruction::addEdge(const RTL *from, const RTL *to)
{
    m_edges.push_back({ from, to });
}


RTL *LiftedInstruction::getFirstRTL()
{
    return !m_rtls.empty() ? m_rtls.front().get() : nullptr;
}


const RTL *LiftedInstruction::getFirstRTL() const
{
    return !m_rtls.empty() ? m_rtls.front().get() : nullptr;
}
