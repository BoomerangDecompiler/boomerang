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


#include "boomerang/db/exp/ExpHelp.h"

#include <set>

class QTextStream;


/// Like \ref StatementSet, but the Statements are known to be Assigns,
/// and are sorted sensibly
class AssignSet : public std::set<Assign *, lessAssign>
{
public:
    /// Set union.
    /// *this = *this union other
    void makeUnion(const AssignSet& other);

    /// Set difference.
    /// *this = *this - other
    void makeDiff(const AssignSet& other);

    /// Set intersection.
    /// *this = *this isect other
    void makeIsect(const AssignSet& other);

    /// \returns true if all elements of this set are in \p other
    bool isSubSetOf(const AssignSet& other);

    /// Remove this Assign.
    /// \returns false if it was not found.
    bool remove(Assign *asgn);

    /// \returns true if found.
    bool exists(Assign *asgn);

    /// \returns true if any assignment in this set defines \p loc
    bool definesLoc(SharedExp loc) const;

    /// Find a definition for \p loc on the LHS in this Assign set.
    /// If found, return pointer to the Assign with that LHS (else return nullptr)
    Assign *lookupLoc(SharedExp loc);

    bool operator<(const AssignSet& o) const;

    void print(QTextStream& os) const;

    /// Print just the numbers to stream os
    void printNums(QTextStream& os);

    /// Print to a string, for debugging
    char *prints();

    /// Print to standard error for debugging
    void dump();
};
