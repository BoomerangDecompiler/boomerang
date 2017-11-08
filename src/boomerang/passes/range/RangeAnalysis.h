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


#include "boomerang/passes/Pass.h"

class Cfg;
class Function;
class Statement;
class UserProc;

/**
 * Detect and log possible buffer overflows
 */
class RangeAnalysisPass : public FunctionPass
{
    friend class RangeVisitor;

public:
    RangeAnalysisPass();

    /// Range analysis (for procedure).
    bool runOnFunction(Function& function) override;

private:
    /// Add Junction statements
    void addJunctionStatements(Cfg& cfg);
    void clearRanges();
    void logSuspectMemoryDefs(UserProc& UF);

    class RangePrivateData *m_rangeData;
};
