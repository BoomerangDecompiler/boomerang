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

    /// Remove unused or redundant parameters and return values from the program.
    /// \returns true if any change
    bool removeUnusedParamsAndReturns();

    /// Have to transform out of SSA form after the above final pass
    /// Convert from SSA form
    void fromSSAForm();

private:
    Prog *m_prog;
};
