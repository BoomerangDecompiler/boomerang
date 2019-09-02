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
    void visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(const std::shared_ptr<BranchStatement> &stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(const std::shared_ptr<CallStatement> &stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    void visit(const std::shared_ptr<ReturnStatement> &stmt, bool &visitChildren) override;

private:
    /// Code common to DFA type recovery of assignments
    void visitAssignment(const std::shared_ptr<Assignment> &stmt, bool &visitChildren);

private:
    bool m_changed = false;
};
