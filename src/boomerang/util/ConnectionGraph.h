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

#include <vector>


/**
 * A class to store connections in a graph, e.g. for interferences of types or live ranges, or the phi_unite relation
 * that phi statements imply
 * If a is connected to b, then b is automatically connected to a
 * \internal This is implemented in a std::multimap, even though Appel suggests a bitmap (e.g. std::vector<bool> does this in a
 * space efficient manner), but then you still need maps from expression to bit number. So here a standard map is used,
 * and when a -> b is inserted, b->a is redundantly inserted.
 */
class ConnectionGraph
{
    std::multimap<SharedExp, SharedExp, lessExpStar> emap;   ///< The map

public:
    typedef std::multimap<SharedExp, SharedExp, lessExpStar>::iterator       iterator;
    typedef std::multimap<SharedExp, SharedExp, lessExpStar>::const_iterator const_iterator;

    ConnectionGraph() = default;

    /// Add pair with check for existing
    void add(SharedExp a, SharedExp b);
    void connect(SharedExp a, SharedExp b);

    iterator begin()       { return emap.begin(); }
    iterator end()         { return emap.end(); }
    const_iterator begin() const { return emap.begin(); }
    const_iterator end()   const { return emap.end(); }

    /// Return a count of locations connected to \a e
    int count(SharedExp a) const;

    /// Return true if a is connected to b
    bool isConnected(SharedExp a, const Exp& b) const;
    bool allRefsHaveDefs() const;

    // Modify the map so that a <-> b becomes a <-> c
    /// Update the map that used to be a <-> b, now it is a <-> c
    void update(SharedExp a, SharedExp b, SharedExp c);

    // Remove the mapping at *aa, and return a valid iterator for looping
    iterator remove(iterator aa); ///< Remove the mapping at *aa
    void dump() const;            ///< Dump for debugging

private:
    std::vector<SharedExp> allConnected(SharedExp a);
};
