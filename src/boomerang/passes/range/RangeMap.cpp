#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RangeMap.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/passes/range/Range.h"


bool RangeMap::empty() const
{
    return m_ranges.empty();
}


void RangeMap::addRange(SharedExp loc, Range& r)
{
    m_ranges[loc] = r;
}


bool RangeMap::hasRange(const SharedExp& loc)
{
    return m_ranges.find(loc) != m_ranges.end();
}


Range& RangeMap::getRange(const SharedExp& loc)
{
    if (m_ranges.find(loc) == m_ranges.end()) {
        return *(new Range(1, Range::MIN, Range::MAX, Const::get(0)));
    }

    return m_ranges[loc];
}


void RangeMap::unionWith(RangeMap& other)
{
    for (auto& elem : other.m_ranges) {
        if (m_ranges.find((elem).first) == m_ranges.end()) {
            m_ranges[(elem).first] = (elem).second;
        }
        else {
            m_ranges[(elem).first].unionWith((elem).second);
        }
    }
}


void RangeMap::widenWith(RangeMap& other)
{
    for (auto& elem : other.m_ranges) {
        if (m_ranges.find((elem).first) == m_ranges.end()) {
            m_ranges[(elem).first] = (elem).second;
        }
        else {
            m_ranges[(elem).first].widenWith((elem).second);
        }
    }
}


QString RangeMap::toString() const
{
    QStringList res;

    for (const std::pair<SharedExp, Range>& elem : m_ranges) {
        res += QString("%1 -> %2").arg(elem.first->toString()).arg(elem.second.toString());
    }

    return res.join(", ");
}


SharedExp RangeMap::substInto(SharedExp e, ExpSet *only) const
{
    bool changes;
    int  count = 0;

    do {
        changes = false;

        for (const std::pair<SharedExp, Range>& elem : m_ranges) {
            if (only && (only->find(elem.first) == only->end())) {
                continue;
            }

            bool      change = false;
            SharedExp eold   = nullptr;

            if (DEBUG_RANGE_ANALYSIS) {
                eold = e->clone();
            }

            if (elem.second.getLowerBound() == elem.second.getUpperBound()) {
                // The following likely contains a bug, the replace argument is using direct pointer to
                // elem.second.getBase() instead of a clone.
                e = e->searchReplaceAll(*elem.first,
                                        (Binary::get(opPlus, elem.second.getBase(),
                                                     Const::get(elem.second.getLowerBound())))->simplify(),
                                        change);
            }

            if (change) {
                e = e->simplify()->simplifyArith();

                if (DEBUG_RANGE_ANALYSIS) {
                    LOG_VERBOSE("Applied %1 to %2 to get %3", elem.first, eold, e);
                }

                changes = true;
            }
        }

        count++;
        assert(count < 5);
    } while (changes);

    return e;
}


void RangeMap::killAllMemOfs()
{
    for (auto& elem : m_ranges) {
        if ((elem).first->isMemOf()) {
            Range emptyRange;
            (elem).second.unionWith(emptyRange);
        }
    }
}


bool RangeMap::isSubset(RangeMap& other) const
{
    for (std::pair<SharedExp, Range> it : m_ranges) {
        if (other.m_ranges.find(it.first) == other.m_ranges.end()) {
            if (DEBUG_RANGE_ANALYSIS) {
                LOG_VERBOSE("Could not find %1 in other, not a subset", it.first);
            }

            return false;
        }

        Range& r = other.m_ranges[it.first];

        if (!(it.second == r)) {
            if (DEBUG_RANGE_ANALYSIS) {
                LOG_VERBOSE("Range for %1 in other %2 is not equal to range in this %3, not a subset",
                            it.first, r, it.second);
            }

            return false;
        }
    }

    return true;
}

