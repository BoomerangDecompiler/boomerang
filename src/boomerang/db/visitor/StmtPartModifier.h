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


/**
 * Specialised for propagating to. The top level of the lhs
 * of assignment-like statements (including arguments in calls)
 * is not modified. So for example eax := ebx -> eax := local2,
 * but in m[xxx] := rhs, the rhs and xxx are modified,
 * but not the m[xxx]
 */
class StmtPartModifier
{
    bool m_ignoreCol;

public:
    ExpModifier *mod; ///< The expression modifier object
    StmtPartModifier(ExpModifier *em, bool ignoreCol = false)
        : m_ignoreCol(ignoreCol)
        , mod(em) {}

    virtual ~StmtPartModifier() = default;
    bool ignoreCollector() const { return m_ignoreCol; }

    // This class' visitor functions don't return anything. Maybe we'll need return values at a later stage.
    virtual void visit(Assign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(PhiAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ImplicitAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(BoolAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(GotoStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(BranchStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(CaseStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(CallStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ReturnStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ImpRefStatement * /*s*/, bool& recur) { recur = true; }
};
