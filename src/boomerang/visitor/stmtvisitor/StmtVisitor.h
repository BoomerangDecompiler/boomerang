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
    virtual bool visit(const RTL *rtl);

    /// Visit this statement.
    /// \returns true to continue visiting.
    virtual bool visit(const Assign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const PhiAssign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const ImplicitAssign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const BoolAssign *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const GotoStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const BranchStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const CaseStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const CallStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const ReturnStatement *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(const ImpRefStatement *stmt);
};
