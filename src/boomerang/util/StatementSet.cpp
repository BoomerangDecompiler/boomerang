#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementSet.h"


#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/statements/Statement.h"


#include <QTextStream>


QTextStream& operator<<(QTextStream& os, const StatementSet *ss)
{
    ss->print(os);
    return os;
}


void StatementSet::makeUnion(const StatementSet& other)
{
    std::set<Statement *>::iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        insert(*it);
    }
}


void StatementSet::makeDiff(const StatementSet& other)
{
    std::set<Statement *>::iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        erase(*it);
    }
}


void StatementSet::makeIsect(const StatementSet& other)
{
    std::set<Statement *>::iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            // Not in both sets
            erase(it);
        }
    }
}


bool StatementSet::isSubSetOf(const StatementSet& other)
{
    std::set<Statement *>::iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            return false;
        }
    }

    return true;
}


bool StatementSet::remove(Statement *s)
{
    if (find(s) != end()) {
        erase(s);
        return true;
    }

    return false;
}


bool StatementSet::exists(Statement *s)
{
    iterator it = find(s);

    return(it != end());
}


bool StatementSet::definesLoc(SharedExp loc)
{
    for (auto const& elem : *this) {
        if ((elem)->definesLoc(loc)) {
            return true;
        }
    }

    return false;
}


const char *StatementSet::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    std::set<Statement *>::iterator it;

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


void StatementSet::dump()
{
    QTextStream q_cerr(stderr);

    print(q_cerr);
}


void StatementSet::print(QTextStream& os) const
{
    std::set<Statement *>::iterator it;

    for (it = begin(); it != end(); it++) {
        if (it != begin()) {
            os << ",\t";
        }

        os << *it;
    }

    os << "\n";
}


void StatementSet::printNums(QTextStream& os)
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


bool StatementSet::operator<(const StatementSet& o) const
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
