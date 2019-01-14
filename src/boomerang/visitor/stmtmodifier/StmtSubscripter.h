#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang/visitor/stmtmodifier/StmtModifier.h"


class ExpSubscripter;


class BOOMERANG_API StmtSubscripter : public StmtModifier
{
public:
    StmtSubscripter(ExpSubscripter *es);
    virtual ~StmtSubscripter() override = default;

public:
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
};
