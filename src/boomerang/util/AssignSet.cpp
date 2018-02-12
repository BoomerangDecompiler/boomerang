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


void AssignSet::makeUnion(const AssignSet& other)
{
    for (Assign *as : other) {
        insert(as);
    }
}


void AssignSet::makeDiff(const AssignSet& other)
{
    for (Assign *as : other) {
        this->erase(as);
    }
}


void AssignSet::makeIsect(const AssignSet& other)
{
    for (iterator it = begin(); it != end();) {
        if (other.find(*it) == other.end()) {
            // Not in both sets
            it = erase(it);
        }
        else {
            ++it;
        }
    }
}


bool AssignSet::isSubSetOf(const AssignSet& other)
{
    for (auto it = begin(); it != end(); ++it) {
        if (other.find(*it) == other.end()) {
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
    bool first = true;

    for (Assign *as : *this) {
        if (first) {
            first = false;
        }
        else {
            ost << ",\t";
        }

        ost << as;
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
    for (iterator it = begin(); it != end(); ++it) {
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
    if (size() != o.size()) {
        return size() < o.size();
    }

    for (auto it1 = begin(), it2 = o.begin(); it1 != end(); ++it1, ++it2) {
        if (*it1 != *it2) {
            return *it1 < *it2;
        }
    }

    return false;
}
