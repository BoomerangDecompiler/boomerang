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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/db/proc/UserProc.h"

#include <unordered_map>


/**
 * Contains the algorithm that determines how and in which order UserProcs are decompiled.
 */
class BOOMERANG_API ProcDecompiler
{
public:
    ProcDecompiler();

public:
    void decompileRecursive(UserProc *proc);

private:
    ProcStatus tryDecompileRecursive(UserProc *proc);

    void createRecursionGoup(const std::shared_ptr<ProcSet> &newGroup);
    void addToRecursionGroup(UserProc *proc, const std::shared_ptr<ProcSet> &recursionGroup);

private:
    /// Early decompile:
    /// sort CFG, number statements, dominator tree, place phi functions, number statements, first
    /// rename, propagation: ready for preserveds.
    void earlyDecompile(UserProc *proc);

    /// Middle decompile: All the decompilation from preservation up to
    /// but not including removing unused statements.
    /// \returns the cycle set from the recursive call to decompile()
    void middleDecompile(UserProc *proc);

    /// Analyse the whole group of procedures for conditional preserveds, and update till no change.
    /// Also finalise the whole group.
    void recursionGroupAnalysis(const std::shared_ptr<ProcSet> &callStack);

    /// \returns true if any change
    bool decompileProcInRecursionGroup(UserProc *proc, ProcSet &visited);

    /// Remove unused statements etc.
    void lateDecompile(UserProc *proc);

    void printCallStack();

    /**
     * Copy the RTLs for the already decoded Indirect Control Transfer instructions,
     * and decode any new targets in this CFG.
     *
     * Note that we have to delay the new target decoding till now,
     * because otherwise we will attempt to decode nested switch statements
     * without having any SSA renaming, propagation, etc
     */
    void saveDecodedICTs(UserProc *proc);

    /**
     * Re-decompile \p proc from scratch. The proc must be at the top of the call stack
     * (i.e. the one that is currently decompiled).
     */
    ProcStatus reDecompileRecursive(UserProc *proc);

    /**
     * Tries to convert indirect call statements to direct call statements.
     * If successfully converted, decompiles the callee recursively.
     */
    bool tryConvertCallsToDirect(UserProc *proc);

    /**
     * Tries to find and decompile functions whose addresses are assigned to function pointers,
     * e.g. for
     *  *func* foo := 0x08049190
     * we can decompile the address 0x08049190.
     */
    bool tryConvertFunctionPointerAssignments(UserProc *proc);

    /**
     * Tries to decompile the function starting at address \p addr.
     * Does not decompile library functions.
     * If the function does not exist, it is created.
     * \returns the new function.
     */
    Function *tryDecompileRecursive(Address entryAddr, Prog *prog);

private:
    ProcList m_callStack;

    /**
     * Pointer to a set of procedures involved in a recursion group.
     * The procedures in the ProcSet form a strongly connected component of the call graph.
     * Each procedure in the recursion group points to the same ProcSet.
     * Procedures not involved in recursion are not present in this map.
     * \note Since strongly connected components are disjunct,
     * each procedure is part of at most 1 recursion group.
     */
    std::unordered_map<UserProc *, std::shared_ptr<ProcSet>> m_recursionGroups;
};
