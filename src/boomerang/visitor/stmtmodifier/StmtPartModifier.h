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


class ExpModifier;
class Assign;
class PhiAssign;
class ImplicitAssign;
class BoolAssign;
class GotoStatement;
class BranchStatement;
class CaseStatement;
class CallStatement;
class ReturnStatement;


/**
 * Specialised for propagating to. The top level of the lhs
 * of assignment-like statements (including arguments in calls)
 * is not modified. So for example eax := ebx -> eax := local2,
 * but in m[xxx] := rhs, the rhs and xxx are modified,
 * but not the m[xxx]
 *
 * \note This class' visitor functions don't return anything. Maybe we'll need return values at a
 * later stage.
 */
class StmtPartModifier
{
public:
    StmtPartModifier(ExpModifier *em, bool ignoreCol = false);
    virtual ~StmtPartModifier() = default;

public:
    bool ignoreCollector() const { return m_ignoreCol; }

    /// Visit this statement.
    /// \param[in] stmt Statement to visit
    /// \param[out] visitChildren set to true to visit children of this statement
    virtual void visit(Assign *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(PhiAssign *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(ImplicitAssign *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(BoolAssign *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(GotoStatement *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(BranchStatement *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(CaseStatement *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(CallStatement *stmt, bool &visitChildren);

    /// \copydoc StmtPartModifier::visit
    virtual void visit(ReturnStatement *stmt, bool &visitChildren);

public:
    ExpModifier *mod; ///< The expression modifier object

private:
    bool m_ignoreCol;
};
