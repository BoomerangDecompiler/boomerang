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


class Cfg;


class CFGCompressor
{
public:
    /**
     * Given a well-formed cfg, optimizations are performed on the graph
     * to reduce the number of basic blocks and edges. Assumes that \p cfg
     * is well-formed.
     *
     * Optimizations performed are:
     *  - Removal of redundant jumps (e.g. remove J in A->J->B if J only contains a jump)
     *  - Removal of BBs not reachable from the entry BB.
     *
     * \sa Cfg::isWellFormed
     * \returns true if the Cfg was changed.
     */
    bool compressCFG(Cfg *cfg);

private:
    /// Removes BBs that are not reachable from the entry BB.
    bool removeOrphanBBs(Cfg *cfg);

};
