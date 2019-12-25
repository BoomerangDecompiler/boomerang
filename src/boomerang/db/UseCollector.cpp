#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UseCollector.h"

#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/Util.h"
#include "boomerang/visitor/expmodifier/ExpSSAXformer.h"


UseCollector::UseCollector()
{
}


UseCollector::~UseCollector()
{
}


bool UseCollector::operator==(const UseCollector &other) const
{
    if (other.m_locs.size() != m_locs.size()) {
        return false;
    }

    iterator it1, it2;
    for (it1 = m_locs.begin(), it2 = other.m_locs.begin(); it1 != m_locs.end(); ++it1, ++it2) {
        if (!(**it1 == **it2)) {
            return false;
        }
    }

    return true;
}


void UseCollector::makeCloneOf(const UseCollector &other)
{
    m_locs.clear();

    for (auto const &elem : other) {
        m_locs.insert((elem)->clone());
    }
}


void UseCollector::clear()
{
    m_locs.clear();
}


void UseCollector::collectUse(SharedExp e)
{
    m_locs.insert(e);
}


void UseCollector::print(OStream &os) const
{
    if (m_locs.empty()) {
        os << "<None>";
        return;
    }

    bool first = true;

    for (auto const &elem : m_locs) {
        if (first) {
            first = false;
        }
        else {
            os << ",  ";
        }

        (elem)->print(os);
    }
}


void UseCollector::fromSSAForm(UserProc *proc, const SharedStmt &def)
{
    LocationSet removes, inserts;
    iterator it;
    ExpSSAXformer esx(proc);

    for (it = m_locs.begin(); it != m_locs.end(); ++it) {
        auto ref      = RefExp::get(*it, def); // Wrap it in a def
        SharedExp ret = ref->acceptModifier(&esx);

        // If there is no change, ret will equal *it again (i.e. fromSSAForm just removed the
        // subscript)
        if (ret != *it) { // Pointer comparison
            // There was a change; we want to replace *it with ret
            removes.insert(*it);
            inserts.insert(ret);
        }
    }

    for (it = removes.begin(); it != removes.end(); ++it) {
        m_locs.remove(*it);
    }

    for (it = inserts.begin(); it != inserts.end(); ++it) {
        m_locs.insert(*it);
    }
}


void UseCollector::removeUse(SharedExp loc)
{
    m_locs.remove(loc);
}


UseCollector::iterator UseCollector::removeUse(UseCollector::iterator it)
{
    return m_locs.erase(it);
}
