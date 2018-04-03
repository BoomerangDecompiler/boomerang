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


class RTL;
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
 * The StmtVisitor class is used for code that has to work with all the Statement classes.
 * One advantage is that you don't need to declare a function in every class
 * derived from Statement: the accept methods already do that for you.
 * It does not automatically visit the expressions in the statement.
 */
class StmtVisitor
{
public:
    StmtVisitor() = default;
    virtual ~StmtVisitor() = default;

public:
    /// returns true to continue iterating the container
    virtual bool visit(RTL *rtl);

    /// Visit this statement.
    /// \returns true to continue visiting.
    virtual bool visit(Assign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(PhiAssign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(ImplicitAssign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(BoolAssign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(GotoStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(BranchStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(CaseStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(CallStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(ReturnStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(ImpRefStatement *stmt);
};
