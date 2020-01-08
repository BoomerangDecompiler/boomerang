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


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/ExpHelp.h"

#include <set>


class Prog;


class UnusedReturnRemover
{
public:
    explicit UnusedReturnRemover(Prog *prog);

public:
    /**
     * Remove unused return locations.
     * This is the global removing of unused and redundant returns. The initial idea
     * is simple enough: remove some returns according to the formula:
     * returns(p) = modifieds(p) isect union(live at c) for all c calling p.
     *
     * However, removing returns reduces the uses, leading to three effects:
     * 1) The statement that defines the return, if only used by that return, becomes unused
     * 2) if the return is implicitly defined, then the parameters may be reduced, which affects all
     * callers 3) if the return is defined at a call, the location may no longer be live at the
     * call. If not, you need to check the child, and do the union again (hence needing a list of
     * callers) to find out if this change also affects that child. \returns true if any change
     */
    bool removeUnusedReturns();

private:
    /**
     * Remove any returns that are not used by any callers
     *
     * Remove unused returns for this procedure, based on the equation:
     * returns = modifieds isect union(live at c) for all c calling this procedure.
     * The intersection operation will only remove locations. Removing returns can have three
     * effects for each component y used by that return (e.g. if return r24 := r25{10} + r26{20} is
     * removed, statements 10 and 20 will be affected and y will take the values r25{10} and
     * r26{20}): 1) a statement s defining a return becomes unused if the only use of its definition
     * was y 2) a call statement c defining y will no longer have y live if the return was the only
     * use of y. This could cause a change to the returns of c's destination, so
     * removeRedundantReturns has to be called for c's destination proc (if it turns out to be the
     * only definition, and that proc was not already scheduled for return removing). 3) if y is a
     * parameter (i.e. y is of the form loc{0}), then the signature of this procedure changes, and
     * all callers have to have their arguments trimmed, and a similar process has to be applied to
     * all those caller's removed arguments as is applied here to the removed returns.
     *
     * The \a removeRetSet is the set of procedures to process with this logic; caller in Prog calls
     * all elements in this set (only add procs to this set, never remove)
     *
     * \returns true if any change
     */
    bool removeUnusedParamsAndReturns(UserProc *proc);

    /**
     * Update parameters and call livenesses to take into account the changes
     * caused by removing a return from this procedure, or a callee's parameter
     * (which affects this procedure's arguments, which are also uses).
     *
     * Need to save the old parameters and call livenesses, redo the dataflow and
     * removal of unused statements, recalculate the parameters and call livenesses,
     * and if either or both of these are changed, recurse to parents or those calls'
     * children respectively. (When call livenesses change like this, it means that
     * the recently removed return was the only use of that liveness, i.e. there was a
     * return chain.)
     * \sa removeRedundantReturns().
     */
    void updateForUseChange(UserProc *proc);

    /// Remove returns from the return statement to match the signature of \p proc
    /// \returns true if any change
    bool removeReturnsToMatchSignature(UserProc *proc);

private:
    Prog *m_prog;
    ProcSet m_removeRetSet; ///< UserProcs that need their returns updated
};
