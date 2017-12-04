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
    iterator it;

    for (it = begin(); it != end(); it++) {
        if (*it == s) {
            erase(it);
            return true;
        }
    }

    return false;
}


void StatementList::append(const StatementList& sl)
{
    insert(end(), sl.begin(), sl.end());
}


void StatementList::append(const InstructionSet& ss)
{
    insert(end(), ss.begin(), ss.end());
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
    clear();

    for (auto& elem : a) {
        Assignment *as = (Assignment *)elem;

        if (b.exists(as->getLeft())) {
            push_back(as);
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
        if (*((Assignment *)elem)->getLeft() == *loc) {
            return true;
        }
    }

    return false;
}


void StatementList::removeDefOf(SharedExp loc)
{
    for (iterator it = begin(); it != end(); it++) {
        if (*((Assignment *)*it)->getLeft() == *loc) {
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
        SharedExp left = ((Assignment *)elem)->getLeft();

        if (*left == *loc) {
            return (Assignment *)elem;
        }

        if (left->isLocal()) {
            auto           l = left->access<Location>();
            SharedConstExp e = l->getProc()->expFromSymbol(l->access<Const, 1>()->getStr());

            if (e && ((*e == *loc) || (e->isSubscript() && (*e->getSubExp1() == *loc)))) {
                return (Assignment *)elem;
            }
        }
    }

    return nullptr;
}
