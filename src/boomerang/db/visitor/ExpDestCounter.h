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


#include "boomerang/db/visitor/ExpVisitor.h"

#include "boomerang/db/exp/ExpHelp.h"

/**
 * Count the number of times a reference expression is used. Increments the count multiple times if the same reference
 * expression appears multiple times (so can't use UsedLocsFinder for this)
 */
class ExpDestCounter : public ExpVisitor
{
public:
    typedef std::map<SharedExp, int, lessExpStar> ExpCountMap;

public:
    ExpDestCounter(ExpCountMap& dc);

    bool visit(const std::shared_ptr<RefExp>& e, bool& override) override;

private:
    ExpCountMap& m_destCounts;
};

