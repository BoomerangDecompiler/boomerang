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


#include <QString>


class UserProc;


enum class PassID
{
    INVALID      = -1,
    Dominators   = 0,
    PhiPlacement = 1,
    BlockVarRename,
    CallDefineUpdate,
    CallArgumentUpdate,
    StatementInit,
    GlobalConstReplace,
    StatementPropagation,
    BBSimplify,
    CallAndPhiFix,
    SPPreservation,
    PreservationAnalysis,
    StrengthReductionReversal,
    AssignRemoval,
    DuplicateArgsRemoval,
    CallLivenessRemoval,
    LocalTypeAnalysis,
    BranchAnalysis,
    FromSSAForm,
    FinalParameterSearch,
    UnusedStatementRemoval,
    ParameterSymbolMap,
    UnusedLocalRemoval,
    UnusedParamRemoval,
    ImplicitPlacement,
    LocalAndParamMap,
    NUM_PASSES
};


/// Passes run during the decompilation process
/// and update statements in a UserProc.
class IPass
{
public:
    IPass(const QString &name, PassID type);
    virtual ~IPass() = default;

public:
    const QString &getName() const { return m_name; }
    PassID getType() const { return m_type; }

    /// \returns true iff the pass only accesses statements inside the function.
    /// This means that procLocal passes can be executed for each function in parallel.
    virtual bool isProcLocal() const { return false; }

    /// Run this pass, updating \p proc
    /// \returns true iff any change
    virtual bool execute(UserProc *proc) = 0;

private:
    QString m_name;
    PassID m_type;
};
