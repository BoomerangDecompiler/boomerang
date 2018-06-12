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


class Prog;


class ProgDecompiler
{
public:
    ProgDecompiler(Prog *prog);

public:
    /// Do the main non-global decompilation steps
    void decompile();

private:
    /// Do global type analysis.
    /// \note For now, it just does local type analysis for every procedure of the program.
    void globalTypeAnalysis();

    /// As the name suggests, removes globals unused in the decompiled code.
    void removeUnusedGlobals();

    /**
     * Remove unused return locations.
     * This is the global removing of unused and redundant returns. The initial idea
     * is simple enough: remove some returns according to the formula:
     * returns(p) = modifieds(p) isect union(live at c) for all c calling p.
     *
     * However, removing returns reduces the uses, leading to three effects:
     * 1) The statement that defines the return, if only used by that return, becomes unused
     * 2) if the return is implicitly defined, then the parameters may be reduced, which affects all callers
     * 3) if the return is defined at a call, the location may no longer be live at the call. If not, you need to check
     *    the child, and do the union again (hence needing a list of callers) to find out if this change also affects that
     *    child.
     * \returns true if any change
     */
    bool removeUnusedReturns();

    /// Have to transform out of SSA form after the above final pass
    /// Convert from SSA form
    void fromSSAForm();

private:
    Prog *m_prog;
};
