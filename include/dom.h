/*
 * dom.h: include file for experimental dominance frontier code
 */

#ifndef __DOM_H__
#define __DOM_H__

#include "cfg.h"

struct DOM {
    /* These first three are not from Appel; they map PBBs to indices */
    std::vector<PBB> BBs;               // Pointers to BBs from indices
    std::map<PBB, int> indices;         // Indices from pointers to BBs
    int next;                           // Next index to use
    // If there is a path from a to b in the cfg, then a is an ancestor of b
    // if dfnum[a] < denum[b]
    std::vector<int> dfnum;             // Number set in depth first search
    std::vector<int> semi;              // Semi dominators
    std::vector<int> ancestor;          // Defines the forest that becomes the
                                        // spanning tree
    std::vector<int> idom;              // Immediate dominator
    std::vector<int> samedom;           // ?
    std::vector<int> vertex;            // ?
    std::vector<int> parent;            // ?
    std::vector<int> best;              // Improves ancestorWithLowestSemi
    std::vector<std::set<int> > bucket; // Deferred calculation?
    int N;                              // Current node number in algorithm
    std::vector<std::set<int> > DF;     // The dominance frontiers
};

#endif  // __DOM_H__
