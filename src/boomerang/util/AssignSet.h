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
    ~AssignSet() {}

    // Make this set the union of itself and other
    void makeUnion(AssignSet& other);         ///< Set union

    // Make this set the difference of itself and other
    void makeDiff(AssignSet& other);          ///< Set difference

    /// Make this set the intersection of itself and other
    void makeIsect(AssignSet& other);         ///< Set intersection

    // Check for the subset relation, i.e. are all my elements also in the set
    // other. Effectively (this intersect other) == this
    bool isSubSetOf(AssignSet& other);        ///< Subset relation

    // Remove this Assign. Return false if it was not found
    bool remove(Assign *a);                   ///< Removal; rets false if not found
    bool removeIfDefines(SharedExp given);    ///< Remove if given exp is defined
    bool removeIfDefines(AssignSet& given);   ///< Remove if any given is def'd

    // Search for a in this Assign set. Return true if found
    bool exists(Assign *s);                   ///< Search; returns false if !found

    // Find a definition for loc in this Assign set. Return true if found
    bool definesLoc(SharedExp loc) const;     ///< Search; returns true if any assignment defines loc

    // Find a definition for loc on the LHS in this Assign set. If found, return pointer to the Assign with that LHS
    Assign *lookupLoc(SharedExp loc);         ///< Search for loc on LHS, return ptr to Assign if found

    bool operator<(const AssignSet& o) const; ///< Compare if less

    void print(QTextStream& os) const;        ///< Print to os

    // Print just the numbers to stream os
    void printNums(QTextStream& os);          ///< Print statements as numbers

    // Print to a string, for debugging
    char *prints();                           ///< Print to string (for debug)
    void dump();                              ///< Print to standard error for debugging
};                                            ///< class AssignSet
