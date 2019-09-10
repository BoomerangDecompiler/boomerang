#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementList.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/Assignment.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/StatementSet.h"
#include "boomerang/util/Util.h"

#include <QString>


bool StatementList::remove(const SharedStmt &s)
{
    for (auto it = begin(); it != end(); ++it) {
        if (*it == s) {
            erase(it);
            return true;
        }
    }

    return false;
}


void StatementList::append(const SharedStmt &s)
{
    assert(s);
    m_list.push_back(s);
}


void StatementList::append(const StatementList &sl)
{
    if (&sl == this) {
        const size_t oldSize = m_list.size();
        auto it              = m_list.begin();
        for (size_t i = 0; i < oldSize; i++) {
            m_list.push_back(*it++);
        }
    }
    else {
        m_list.insert(end(), sl.begin(), sl.end());
    }
}


void StatementList::append(const StatementSet &ss)
{
    m_list.insert(end(), ss.begin(), ss.end());
}


QString StatementList::toString() const
{
    QString tgt;
    OStream ost(&tgt);

    for (auto it = m_list.begin(); it != m_list.end(); it++) {
        ost << *it;
        if (std::next(it) != m_list.end()) {
            ost << ",\t";
        }
    }

    return tgt;
}


void StatementList::makeIsect(StatementList &a, LocationSet &b)
{
    if (this == &a) { // *this = *this isect b
        for (auto it = a.begin(); it != a.end();) {
            assert((*it)->isAssignment());
            std::shared_ptr<Assignment> as = (*it)->as<Assignment>();

            if (!b.contains(as->getLeft())) {
                it = m_list.erase(it);
            }
            else {
                it++;
            }
        }
    }
    else { // normal assignment
        clear();
        for (SharedStmt stmt : a) {
            assert(stmt->isAssignment());
            std::shared_ptr<Assignment> as = stmt->as<Assignment>();

            if (b.contains(as->getLeft())) {
                append(as);
            }
        }
    }
}


bool StatementList::existsOnLeft(const SharedExp &loc) const
{
    for (auto &elem : *this) {
        if (*elem->as<Assignment>()->getLeft() == *loc) {
            return true;
        }
    }

    return false;
}


SharedStmt StatementList::removeFirstDefOf(SharedExp loc)
{
    if (!loc) {
        return nullptr;
    }

    for (iterator it = begin(); it != end(); ++it) {
        assert((*it)->isAssignment());
        std::shared_ptr<Assignment> assign = (*it)->as<Assignment>();

        if (*assign->getLeft() == *loc) {
            erase(it);
            return assign;
        }
    }

    return nullptr;
}


std::shared_ptr<const Assignment> StatementList::findOnLeft(SharedConstExp loc) const
{
    if (empty()) {
        return nullptr;
    }

    for (const auto &elem : *this) {
        assert(elem->isAssignment());
        SharedConstExp left = elem->as<Assignment>()->getLeft();

        if (*left == *loc) {
            return elem->as<const Assignment>();
        }

        if (left->isLocal()) {
            auto l           = left->access<Location>();
            SharedConstExp e = l->getProc()->expFromSymbol(l->access<Const, 1>()->getStr());

            if (e && ((*e == *loc) || (e->isSubscript() && (*e->getSubExp1() == *loc)))) {
                return elem->as<const Assignment>();
            }
        }
    }

    return nullptr;
}


std::shared_ptr<Assignment> StatementList::findOnLeft(SharedExp loc)
{
    if (empty()) {
        return nullptr;
    }

    for (auto &elem : *this) {
        assert(elem->isAssignment());
        SharedConstExp left = elem->as<Assignment>()->getLeft();

        if (*left == *loc) {
            return elem->as<Assignment>();
        }

        if (left->isLocal()) {
            auto l           = left->access<Location>();
            SharedConstExp e = l->getProc()->expFromSymbol(l->access<Const, 1>()->getStr());

            if (e && ((*e == *loc) || (e->isSubscript() && (*e->getSubExp1() == *loc)))) {
                return elem->as<Assignment>();
            }
        }
    }

    return nullptr;
}
