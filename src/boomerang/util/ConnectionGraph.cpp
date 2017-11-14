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


#include "boomerang/db/exp/RefExp.h"
#include "boomerang/util/Log.h"


void ConnectionGraph::add(SharedExp a, SharedExp b)
{
    iterator ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == *b) {
            return; // Don't add a second entry
        }

        ++ff;
    }

    std::pair<SharedExp, SharedExp> pr;
    pr.first  = a;
    pr.second = b;
    emap.insert(pr);
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
    for (auto iter : *this) {
        const SharedExp& fr(iter.first);
        const SharedExp& sc(iter.second);
        assert(std::dynamic_pointer_cast<RefExp>(fr));
        assert(std::dynamic_pointer_cast<RefExp>(sc));

        if (nullptr == std::static_pointer_cast<RefExp>(fr)->getDef()) {
            return false;
        }

        if (nullptr == std::static_pointer_cast<RefExp>(sc)->getDef()) {
            return false;
        }
    }

    return true;
}


void ConnectionGraph::update(SharedExp a, SharedExp b, SharedExp c)
{
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


ConnectionGraph::iterator ConnectionGraph::remove(iterator aa)
{
    assert(aa != emap.end());
    SharedExp b = aa->second;
    emap.erase(aa++);
    iterator bb = emap.find(b);
    assert(bb != emap.end());

    if (bb == aa) {
        ++aa;
    }

    emap.erase(bb);
    return aa;
}


void ConnectionGraph::dump() const
{
    for (auto iter : *this) {
        LOG_MSG("%1 <-> %2", iter.first, iter.second);
    }
}
