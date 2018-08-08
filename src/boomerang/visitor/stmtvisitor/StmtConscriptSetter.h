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


#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"


class BOOMERANG_API StmtConscriptSetter : public StmtVisitor
{
public:
    StmtConscriptSetter(int n, bool clear);
    virtual ~StmtConscriptSetter() = default;

public:
    int getLast() const { return m_curConscript; }

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const Assign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const PhiAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const ImplicitAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const BoolAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const CaseStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const CallStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const ReturnStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const BranchStatement *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const ImpRefStatement *stmt) override;

private:
    int m_curConscript;
    bool m_clear;
};
