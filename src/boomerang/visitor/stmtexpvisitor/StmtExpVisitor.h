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


#include "boomerang/core/BoomerangAPI.h"


#include <memory>


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


/**
 * StmtExpVisitor is a visitor of statements, and of expressions within those expressions.
 * The visiting of expressions (after the current node) is done by an ExpVisitor
 * (i.e. this is a preorder traversal).
 */
class BOOMERANG_API StmtExpVisitor
{
public:
    StmtExpVisitor(ExpVisitor *v, bool ignoreCol = true);
    virtual ~StmtExpVisitor() = default;

public:
    /// Visit all expressions in a statement.
    /// \param[in]  stmt The statement to visit.
    /// \param[out] visitChildren set to false to not visit child expressions.
    /// \returns true to continue visit
    virtual bool visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<GotoStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<BranchStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<CaseStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<CallStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<ReturnStatement> &stmt, bool &visitChildren);

    bool isIgnoreCol() const { return m_ignoreCol; }

public:
    ExpVisitor *ev;

private:
    bool m_ignoreCol; ///< True if ignoring collectors
};
