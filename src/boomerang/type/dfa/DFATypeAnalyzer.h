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


#include "boomerang/visitor/stmtmodifier/StmtModifier.h"


class Assignment;


/**
 * This modifier traverses all statements
 * and propagates type information about them.
 */
class DFATypeAnalyzer : public StmtModifier
{
public:
    DFATypeAnalyzer();
    virtual ~DFATypeAnalyzer() = default;

public:
    bool hasChanged() const { return m_changed; }
    void resetChanged() { m_changed = false; }

public:
    /// \copydoc StmtModifier::visit
    /// For x0 := phi(x1, x2, ...) want
    /// Tx0 := Tx0 meet (Tx1 meet Tx2 meet ...)
    /// Tx1 := Tx1 meet Tx0
    /// Tx2 := Tx2 meet Tx0
    void visit(PhiAssign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(Assign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(BoolAssign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(BranchStatement *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(CallStatement *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(ImplicitAssign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(ReturnStatement *stmt, bool &visitChildren) override;

private:
    /// Code common to DFA type recovery of assignments
    void visitAssignment(Assignment *stmt, bool &visitChildren);

private:
    bool m_changed = false;
};
