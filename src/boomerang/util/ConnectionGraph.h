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


#include "boomerang/ssl/exp/ExpHelp.h"

#include <map>
#include <vector>


/**
 * A class to store connections in an undirected graph, e.g. for interferences
 * of types or live ranges, or the phi_unite relation that phi statements imply.
 *
 * \internal This is implemented in a std::multimap, even though Appel suggests a bitmap
 * (e.g. std::vector<bool> does this in a space efficient manner),
 * but then you still need maps from expression to bit number.
 * So here a standard map is used, and when a -> b is inserted, b->a is redundantly inserted.
 */
class BOOMERANG_API ConnectionGraph
{
    typedef std::multimap<SharedExp, SharedExp, lessExpStar> ExpExpMap;

public:
    typedef ExpExpMap::iterator       iterator;
    typedef ExpExpMap::const_iterator const_iterator;
    typedef ExpExpMap::reverse_iterator reverse_iterator;
    typedef ExpExpMap::const_reverse_iterator const_reverse_iterator;

public:
    iterator begin() { return emap.begin(); }
    iterator end()   { return emap.end(); }
    const_iterator begin() const { return emap.begin(); }
    const_iterator end()   const { return emap.end(); }

    reverse_iterator rbegin() { return emap.rbegin(); }
    reverse_iterator rend()   { return emap.rend();   }

    const_reverse_iterator rbegin() const { return emap.rbegin(); }
    const_reverse_iterator rend()   const { return emap.rend();   }

public:
    /// Add pair with check for existing
    /// \returns true if successfully inserted
    bool add(SharedExp a, SharedExp b);

    /**
     * Connect all neighbours of \p a to \p b and
     * connect all neighbours of \p b to \p a
     */
    void connect(SharedExp a, SharedExp b);

    /// Return true if a is connected to b
    bool isConnected(SharedExp a, const Exp& b) const;

    /// Return the number of expression connected to \p a
    int count(SharedExp a) const;

    /**
     * For all \ref RefExp expression in this graph,
     * check if they have definitions.
     */
    bool allRefsHaveDefs() const;

    /**
     * Modify the graph so that \p a <-> \p b becomes \p a <-> \p c
     */
    void updateConnection(SharedExp a, SharedExp b, SharedExp c);

private:
    std::vector<SharedExp> allConnected(SharedExp a);

private:
   ExpExpMap emap;   ///< The map
};
