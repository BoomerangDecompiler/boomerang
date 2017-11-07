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
class JunctionStatement;
class ImpRefStatement;


/**
 * The StmtVisitor class is used for code that has to work with all the Statement classes. One advantage is that you
 * don't need to declare a function in every class derived from Statement: the accept methods already do that for you.
 * It does not automatically visit the expressions in the statement.
 */
class StmtVisitor
{
public:
    StmtVisitor() = default;
    virtual ~StmtVisitor() = default;

    // visitor functions,
    // returns true to continue iterating the container
    virtual bool visit(RTL *rtl); // By default, visits all statements

    virtual bool visit(Assign *stmt);
    virtual bool visit(PhiAssign *stmt);
    virtual bool visit(ImplicitAssign *stmt);
    virtual bool visit(BoolAssign *stmt);
    virtual bool visit(GotoStatement *stmt);
    virtual bool visit(BranchStatement *stmt);
    virtual bool visit(CaseStatement *stmt);
    virtual bool visit(CallStatement *stmt);
    virtual bool visit(ReturnStatement *stmt);
    virtual bool visit(ImpRefStatement *stmt);
    virtual bool visit(JunctionStatement *stmt);
};
