#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang/db/visitor/StmtModifier.h"


class ExpSubscripter;


class StmtSubscripter : public StmtModifier
{
public:
    StmtSubscripter(ExpSubscripter *es);
    virtual ~StmtSubscripter() override = default;

    virtual void visit(Assign *s, bool& recur) override;
    virtual void visit(PhiAssign *s, bool& recur) override;
    virtual void visit(ImplicitAssign *s, bool& recur) override;
    virtual void visit(BoolAssign *s, bool& recur) override;
    virtual void visit(CallStatement *s, bool& recur) override;
};
