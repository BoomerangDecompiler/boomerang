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


class ProcCFG;
class BasicBlock;


class CFGCompressor
{
public:
    /**
     * Given a well-formed ProcCFG, optimizations are performed on the graph
     * to reduce the number of basic blocks and edges. Assumes that \p cfg
     * is well-formed.
     *
     * Optimizations performed are:
     *  - Removal of redundant jumps (e.g. remove J in A->J->B if J only contains a jump)
     *  - Removal of empty fragments (not containing any semantics), if possible
     *  - Removal of fragments not reachable from the entry fragment.
     *
     * \sa ProcCFG::isWellFormed
     * \returns true if the ProcCFG was changed.
     */
    bool compressCFG(ProcCFG *cfg);

private:
    /// Removes empty jumps and empty BBs.
    bool removeEmptyJumps(ProcCFG *cfg);

    /// Removes fragments that are not reachable from the entry fragment.
    bool removeOrphanFragments(ProcCFG *cfg);
};
