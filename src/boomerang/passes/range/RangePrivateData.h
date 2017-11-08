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


#include <map>

#include "boomerang/passes/range/RangeMap.h"

class Statement;


class RangePrivateData
{
public:
    RangeMap& getRanges(Statement *insn)
    {
        return m_ranges[insn];
    }

    const RangeMap& getRanges(Statement *insn) const
    {
        return m_ranges.at(insn);
    }

    void setRanges(Statement *insn, const RangeMap& r)
    {
        m_ranges[insn] = r;
    }

    void setRanges(Statement *insn, RangeMap&& r)
    {
        m_ranges[insn] = std::move(r);
    }

    void clearRanges()
    {
        m_savedInputRanges.clear();
    }

    RangeMap& getBranchRange(BranchStatement *s)
    {
        return m_branchRanges[s];
    }

    void setBranchRange(BranchStatement *s, const RangeMap& rm)
    {
        m_branchRanges[s] = rm;
    }

    void setBranchRange(BranchStatement *s, RangeMap&& rm)
    {
        m_branchRanges[s] = std::move(rm);
    }

    void setSavedRanges(Statement *insn, RangeMap map);
    RangeMap getSavedRanges(Statement *insn);

private:
    std::map<Statement *, RangeMap> m_savedInputRanges;       ///< overestimation of ranges of locations
    std::map<Statement *, RangeMap> m_ranges;                 ///< saved overestimation of ranges of locations
    std::map<BranchStatement *, RangeMap> m_branchRanges;
};
