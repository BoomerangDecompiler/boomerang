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
    StmtExpVisitor(ExpVisitor *v, bool _ignoreCol = true);
    virtual ~StmtExpVisitor() = default;

    virtual bool visit(Assign * /*stmt*/, bool& override);
    virtual bool visit(PhiAssign * /*stmt*/, bool& override);
    virtual bool visit(ImplicitAssign * /*stmt*/, bool& override);
    virtual bool visit(BoolAssign * /*stmt*/, bool& override);
    virtual bool visit(GotoStatement * /*stmt*/, bool& override);
    virtual bool visit(BranchStatement * /*stmt*/, bool& override);
    virtual bool visit(CaseStatement * /*stmt*/, bool& override);
    virtual bool visit(CallStatement * /*stmt*/, bool& override);
    virtual bool visit(ReturnStatement * /*stmt*/, bool& override);
    virtual bool visit(ImpRefStatement * /*stmt*/, bool& override);

    bool isIgnoreCol() const { return m_ignoreCol; }

public:
    ExpVisitor *ev;

private:
    bool m_ignoreCol; ///< True if ignoring collectors
};
