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


#include "boomerang/db/visitor/StmtVisitor.h"


class Assign;
class Assignment;
class ImplicitAssign;
class BoolAssign;


/**
 *
 */
class StmtCastInserter : public StmtVisitor
{
public:
    StmtCastInserter() = default;

    bool common(Assignment *s);

    virtual bool visit(Assign *s) override;
    virtual bool visit(PhiAssign *s) override;
    virtual bool visit(ImplicitAssign *s) override;
    virtual bool visit(BoolAssign *s) override;
};
