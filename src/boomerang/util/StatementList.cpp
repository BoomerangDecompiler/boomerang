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


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/statements/Assignment.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/StatementSet.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/Util.h"

#include <QString>
#include <QTextStream>


bool StatementList::remove(Statement *s)
{

    for (auto it = begin(); it != end(); ++it) {
        if (*it == s) {
            erase(it);
            return true;
        }
    }

    return false;
}


void StatementList::append(const StatementList& sl)
{
    m_list.insert(end(), sl.begin(), sl.end());
}


void StatementList::append(const StatementSet& ss)
{
    m_list.insert(end(), ss.begin(), ss.end());
}


char *StatementList::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    for (auto& elem : *this) {
        ost << elem << ",\t";
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}



void StatementList::makeIsect(StatementList& a, LocationSet& b)
{
    if (this == &a) { // *this = *this isect b
        for (auto it = a.begin(); it != a.end(); ) {
            Assignment *as = static_cast<Assignment *>(*it);

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
        for (Statement *stmt : a) {
            Assignment *as = static_cast<Assignment *>(stmt);

            if (b.contains(as->getLeft())) {
                push_back(as);
            }
        }
    }
}


void StatementList::makeCloneOf(const StatementList& other)
{
    clear();

    for (auto& stmt : other) {
        push_back((stmt)->clone());
    }
}


bool StatementList::existsOnLeft(const SharedExp& loc) const
{
    for (auto& elem : *this) {
        if (*static_cast<Assignment *>(elem)->getLeft() == *loc) {
            return true;
        }
    }

    return false;
}


void StatementList::removeFirstDefOf(SharedExp loc)
{
    for (iterator it = begin(); it != end(); ++it) {
        if (*static_cast<Assignment *>(*it)->getLeft() == *loc) {
            erase(it);
            return;
        }
    }
}


Assignment *StatementList::findOnLeft(SharedExp loc) const
{
    if (empty()) {
        return nullptr;
    }

    for (auto& elem : *this) {
        SharedExp left = static_cast<Assignment *>(elem)->getLeft();

        if (*left == *loc) {
            return static_cast<Assignment *>(elem);
        }

        if (left->isLocal()) {
            auto           l = left->access<Location>();
            SharedConstExp e = l->getProc()->expFromSymbol(l->access<Const, 1>()->getStr());

            if (e && ((*e == *loc) || (e->isSubscript() && (*e->getSubExp1() == *loc)))) {
                return static_cast<Assignment *>(elem);
            }
        }
    }

    return nullptr;
}
