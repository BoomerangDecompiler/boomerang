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

#include <set>
#include <memory>


class Statement;
class QTextStream;

using SharedExp = std::shared_ptr<class Exp>;


/**
 * A class to implement sets of statements
 */
class StatementSet : public std::set<Statement *>
{
public:
    /// Set union
    void makeUnion(const StatementSet& other);

    /// Set difference
    void makeDiff(const StatementSet& other);

    /// Set intersection
    void makeIsect(const StatementSet& other);

    /// Set subset relation
    bool isSubSetOf(const StatementSet& other);

    /// Remove this Statement.
    /// \returns false if it was not found.
    bool remove(Statement *s);

    /// \returns true if found.
    bool exists(Statement *s);

    /// \returns true if any assignment in this set defines \p loc
    bool definesLoc(SharedExp loc);

    bool operator<(const StatementSet& o) const;

    void print(QTextStream& os) const;

    /// Print just the numbers
    void printNums(QTextStream& os);

    /// Print to string (for debugging)
    const char *prints();

    /// Print to standard error for debugging
    void dump();
};
