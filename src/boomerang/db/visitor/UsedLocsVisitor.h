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


#include "boomerang/db/visitor/StmtExpVisitor.h"


/**
 *
 */
class UsedLocsVisitor : public StmtExpVisitor
{
public:
    UsedLocsVisitor(ExpVisitor *v, bool cc);
    virtual ~UsedLocsVisitor() override = default;

    /// Needs special attention because the lhs of an assignment isn't used
    /// (except where it's m[blah], when blah is used)
    virtual bool visit(Assign *stmt, bool& override) override;
    virtual bool visit(PhiAssign *stmt, bool& override) override;
    virtual bool visit(ImplicitAssign *stmt, bool& override) override;

    // A BoolAssign uses its condition expression, but not its destination (unless it's an m[x], in which case x is
    // used and not m[x])
    virtual bool visit(BoolAssign *stmt, bool& override) override;

    // Returns aren't used (again, except where m[blah] where blah is used), and there is special logic for when the
    // pass is final
    virtual bool visit(CallStatement *stmt, bool& override) override;

    // Only consider the first return when final
    virtual bool visit(ReturnStatement *stmt, bool& override) override;

private:
    bool m_countCol; ///< True to count uses in collectors
};
