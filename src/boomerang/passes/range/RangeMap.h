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


#include "boomerang/util/Util.h"
#include "boomerang/db/exp/ExpHelp.h"
#include "boomerang/passes/range/Range.h"


/**
 *
 */
class RangeMap : public Printable
{
public:
    RangeMap() = default;

    bool empty() const;

    void addRange(SharedExp loc, Range& r);

    bool hasRange(const SharedExp& loc);

    Range& getRange(const SharedExp& loc);

    void unionWith(RangeMap& other);

    void widenWith(RangeMap& other);

    SharedExp substInto(SharedExp e, ExpSet *only = nullptr) const;

    void killAllMemOfs();

    void clear();

    /// return true if this range map is a subset of the other range map
    bool isSubset(RangeMap& other) const;

    QString toString() const override;

    void print() const;

private:
    std::map<SharedExp, Range, lessExpStar> m_ranges;
};
