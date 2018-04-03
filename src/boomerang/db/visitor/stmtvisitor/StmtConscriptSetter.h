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


#include "boomerang/db/visitor/stmtvisitor/StmtVisitor.h"


class StmtConscriptSetter : public StmtVisitor
{
public:
    StmtConscriptSetter(int n, bool clear);
    virtual ~StmtConscriptSetter() = default;

public:
    int getLast() const { return m_curConscript; }

    /// \copydoc StmtVisitor::visit
    virtual bool visit(Assign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(PhiAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(ImplicitAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(BoolAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(CaseStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(CallStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(ReturnStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(BranchStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(ImpRefStatement *stmt) override;

private:
    int m_curConscript;
    bool m_clear;
};
