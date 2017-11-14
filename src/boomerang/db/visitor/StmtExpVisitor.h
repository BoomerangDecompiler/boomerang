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


class ExpVisitor;
class Assign;
class PhiAssign;
class ImplicitAssign;
class BoolAssign;
class GotoStatement;
class BranchStatement;
class CaseStatement;
class CallStatement;
class ReturnStatement;
class ImpRefStatement;


/**
 * StmtExpVisitor is a visitor of statements, and of expressions within those expressions. The visiting of expressions
 * (after the current node) is done by an ExpVisitor (i.e. this is a preorder traversal).
 */
class StmtExpVisitor
{
public:
    StmtExpVisitor(ExpVisitor *v, bool ignoreCol = true);
    virtual ~StmtExpVisitor() = default;

    /// Visit all expressions in a statement.
    /// \param[in]  stmt The statement to visit.
    /// \param[out] visitChildren set to false to not visit child expressions.
    /// \returns true to continue visit
    virtual bool visit(Assign *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(PhiAssign *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(ImplicitAssign *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(BoolAssign *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(GotoStatement *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(BranchStatement *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(CaseStatement *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(CallStatement *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(ReturnStatement *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(ImpRefStatement *stmt, bool& visitChildren);

    bool isIgnoreCol() const { return m_ignoreCol; }

public:
    ExpVisitor *ev;

private:
    bool m_ignoreCol; ///< True if ignoring collectors
};
