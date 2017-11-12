#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "AssignSet.h"


#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/exp/Terminal.h"


QTextStream& operator<<(QTextStream& os, const AssignSet *as)
{
    as->print(os);
    return os;
}



void AssignSet::makeUnion(AssignSet& other)
{
    iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        insert(*it);
    }
}



void AssignSet::makeDiff(AssignSet& other)
{
    iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        erase(*it);
    }
}


void AssignSet::makeIsect(AssignSet& other)
{
    iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            // Not in both sets
            erase(it);
        }
    }
}


bool AssignSet::isSubSetOf(AssignSet& other)
{
    iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            return false;
        }
    }

    return true;
}


bool AssignSet::remove(Assign *a)
{
    if (find(a) != end()) {
        erase(a);
        return true;
    }

    return false;
}


bool AssignSet::exists(Assign *a)
{
    iterator it = find(a);

    return(it != end());
}


bool AssignSet::definesLoc(SharedExp loc) const
{
    Assign as(loc, Terminal::get(opWild));

    return find(&as) != end();
}


Assign *AssignSet::lookupLoc(SharedExp loc)
{
    Assign   as(loc, Terminal::get(opWild));
    iterator ff = find(&as);

    if (ff == end()) {
        return nullptr;
    }

    return *ff;
}


char *AssignSet::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);
    iterator    it;

    for (it = begin(); it != end(); it++) {
        if (it != begin()) {
            ost << ",\t";
        }

        ost << *it;
    }

    ost << "\n";
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void AssignSet::dump()
{
    QTextStream q_cerr(stderr);

    print(q_cerr);
}


void AssignSet::print(QTextStream& os) const
{
    iterator it;

    for (it = begin(); it != end(); it++) {
        if (it != begin()) {
            os << ",\t";
        }

        os << *it;
    }

    os << "\n";
}


void AssignSet::printNums(QTextStream& os)
{
    for (iterator it = begin(); it != end();) {
        if (*it) {
            (*it)->printNum(os);
        }
        else {
            os << "-"; // Special case for nullptr definition
        }

        if (++it != end()) {
            os << " ";
        }
    }
}


bool AssignSet::operator<(const AssignSet& o) const
{
    if (size() < o.size()) {
        return true;
    }

    if (size() > o.size()) {
        return false;
    }

    const_iterator it1, it2;

    for (it1 = begin(), it2 = o.begin(); it1 != end(); it1++, it2++) {
        if (*it1 < *it2) {
            return true;
        }

        if (*it1 > *it2) {
            return false;
        }
    }

    return false;
}
