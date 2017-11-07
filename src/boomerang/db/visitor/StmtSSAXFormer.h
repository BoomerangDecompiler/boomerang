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


#include "boomerang/db/visitor/StmtModifier.h"

class Assignment;
class ExpSsaXformer;
class UserProc;


/**
 *
 */
class StmtSsaXformer : public StmtModifier
{
public:
    StmtSsaXformer(ExpSsaXformer *esx, UserProc *p);

    void commonLhs(Assignment *s);

    // TODO: find out if recur should, or should not be set ?
    virtual void visit(Assign *s, bool& recur) override;
    virtual void visit(PhiAssign *s, bool& recur) override;
    virtual void visit(ImplicitAssign *s, bool& recur) override;
    virtual void visit(BoolAssign *s, bool& recur) override;
    virtual void visit(CallStatement *s, bool& recur) override;

private:
    UserProc *m_proc;
};
