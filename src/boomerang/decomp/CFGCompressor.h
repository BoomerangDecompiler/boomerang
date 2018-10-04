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
     *  - Removal of empty BBs (not containing any semantics), if possible
     *  - Removal of BBs not reachable from the entry BB.
     *
     * \sa ProcCFG::isWellFormed
     * \returns true if the ProcCFG was changed.
     */
    bool compressCFG(ProcCFG *cfg);

private:
    /// Removes empty jumps and empty BBs.
    bool removeEmptyJumps(ProcCFG *cfg);

    /// \returns true iff \p bb does not contain any statements.
    /// \note This is different from a BB that does not contain
    /// any RTLs, since all RTLs could be empty.
    bool isEmptyBB(const BasicBlock *bb) const;

    /// \returns true iff \p bb only contains an unconditional jump statement.
    bool isEmptyJump(const BasicBlock *bb) const;

    /// Removes BBs that are not reachable from the entry BB.
    bool removeOrphanBBs(ProcCFG *cfg);
};
