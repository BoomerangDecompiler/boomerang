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


#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"


/**
 *
 */
class UsedLocsVisitor : public StmtExpVisitor
{
public:
    UsedLocsVisitor(ExpVisitor *v, bool countCol);
    virtual ~UsedLocsVisitor() = default;

public:
    /// \copydoc StmtExpVisitor::visit
    /// Needs special attention because the lhs of an assignment isn't used
    /// (except where it's m[blah], when blah is used)
    bool visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    bool visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    bool visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    /// A BoolAssign uses its condition expression, but not its destination (unless it's an m[x], in
    /// which case x is used and not m[x])
    bool visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    /// Returns aren't used (again, except where m[blah] where blah is used),
    /// and there is special logic for when the pass is final
    bool visit(const std::shared_ptr<CallStatement> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    /// Only consider the first return when final
    bool visit(const std::shared_ptr<ReturnStatement> &stmt, bool &visitChildren) override;

private:
    bool m_countCol; ///< True to count uses in collectors
};
