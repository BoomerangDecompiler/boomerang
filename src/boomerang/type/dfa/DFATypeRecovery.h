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


#include "boomerang/type/TypeRecovery.h"


class Cfg;
class Signature;
class Statement;
class StatementList;
class UserProc;

/**
 * Data-flow based type recovery.
 */
class DFATypeRecovery : public TypeRecoveryCommon
{
public:
    /// \copydoc ITypeRecovery::recoverFunctionTypes
    void recoverFunctionTypes(Function *function) override;

    /// \copydoc ITypeRecovery::getName
    QString getName() override { return "data-flow based"; }

private:
    void dfaTypeAnalysis(UserProc *proc);
    bool dfaTypeAnalysis(Signature *signature, Cfg *cfg);
    bool dfaTypeAnalysis(Statement *stmt);

    void dumpResults(StatementList& stmts, int iter);

    void dfa_analyze_scaled_array_ref(Statement *s);

    // 3) Check implicit assigns for parameter and global types.
    void dfa_analyze_implict_assigns(Statement *s);
};
