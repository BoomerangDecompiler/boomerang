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


#include "boomerang/visitor/stmtmodifier/StmtModifier.h"


class Assignment;
class ExpSSAXformer;
class UserProc;


/**
 *
 */
class StmtSSAXformer : public StmtModifier
{
public:
    StmtSSAXformer(ExpSSAXformer *esx, UserProc *proc);
    virtual ~StmtSSAXformer() = default;

public:
    /// Common code for the left hand side of assignments
    void handleCommonLHS(Assignment *stmt);

    /// \copydoc StmtModifier::visit
    virtual void visit(Assign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    virtual void visit(PhiAssign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    virtual void visit(ImplicitAssign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    virtual void visit(BoolAssign *stmt, bool &visitChildren) override;

    /// \copydoc StmtModifier::visit
    virtual void visit(CallStatement *stmt, bool &visitChildren) override;

private:
    UserProc *m_proc;
};
