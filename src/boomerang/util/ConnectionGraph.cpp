#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ConnectionGraph.h"


#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/log/Log.h"


bool ConnectionGraph::add(SharedExp a, SharedExp b)
{
    iterator ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == *b) {
            return false; // Don't add a second entry
        }

        ++ff;
    }

    emap.insert({ a, b });
    emap.insert({ b, a });

    return true;
}


std::vector<SharedExp> ConnectionGraph::allConnected(SharedExp a)
{
    std::vector<SharedExp> res;
    const_iterator         ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        res.push_back(ff->second);
        ++ff;
    }

    return res;
}


void ConnectionGraph::connect(SharedExp a, SharedExp b)
{
    // if a is connected to c,d and e, 'b' should also be connected to c,d and e
    std::vector<SharedExp> a_connections = allConnected(a);
    std::vector<SharedExp> b_connections = allConnected(b);
    add(a, b);

    for (const SharedExp& e : b_connections) {
        add(a, e);
    }

    add(b, a);

    for (SharedExp e : a_connections) {
        add(e, b);
    }
}


int ConnectionGraph::count(SharedExp e) const
{
    const_iterator ff = emap.find(e);
    int            n  = 0;

    while (ff != emap.end() && *ff->first == *e) {
        ++n;
        ++ff;
    }

    return n;
}


bool ConnectionGraph::isConnected(SharedExp a, const Exp& b) const
{
    const_iterator ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == b) {
            return true; // Found the connection
        }

        ++ff;
    }

    return false;
}


bool ConnectionGraph::allRefsHaveDefs() const
{
    for (auto it : *this) {
        const std::shared_ptr<RefExp> ref = std::dynamic_pointer_cast<RefExp>(it.first);

        // we just have to compare the keys
        // since we always have a -> b and b -> a in the map
        if (ref && !ref->getDef()) {
            return false;
        }
    }

    return true;
}


void ConnectionGraph::updateConnection(SharedExp a, SharedExp b, SharedExp c)
{
    assert(a);
    assert(b);
    assert(c);

    // find a->b
    iterator ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == *b) {
            ff->second = c; // Now a->c
            break;
        }

        ++ff;
    }

    // find b -> a
    ff = emap.find(b);

    while (ff != emap.end() && *ff->first == *b) {
        if (*ff->second == *a) {
            emap.erase(ff);
            add(c, a); // Now c->a
            break;
        }

        ++ff;
    }
}
