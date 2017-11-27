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


#include <memory>
#include <list>

class InstructionSet;
class LocationSet;
class Statement;
class Assignment;


using SharedExp = std::shared_ptr<class Exp>;


class StatementList : public std::list<Statement *>
{
public:
    /// Special intersection method: this := a intersect b
    /// A special intersection operator; *this becomes the intersection
    /// of StatementList a (assumed to be a list of Assignment *'s)
    /// with the LocationSet b.
    /// Used for calculating returns for a CallStatement
    void makeIsect(StatementList& a, LocationSet& b);

    void append(Statement *s) { push_back(s); }
    void append(const StatementList& sl);
    void append(const InstructionSet& sl);

    /// \returns false if not found
    bool remove(Statement *s);

    /// Remove the first definition where loc appears on the left
    /// \note statements in this list are assumed to be assignments
    void removeDefOf(SharedExp loc);              ///< Remove definitions of loc

    bool exists(Statement *s);            ///< Search; returns false if not found
    char *prints();                       ///< Print to string (for debugging)
    void dump();                          ///< Print to standard error for debugging
    void makeCloneOf(const StatementList& other);

    /// Return true if loc appears on the left of any statements in this list
    /// Note: statements in this list are assumed to be assignments
    bool existsOnLeft(const SharedExp& loc) const; ///< True if loc exists on the LHS of any Assignment in this list

    /// Find the first Assignment with loc on the LHS
    Assignment *findOnLeft(SharedExp loc) const;   ///< Return the first stmt with loc on the LHS
};

