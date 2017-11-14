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

    bool common(Assignment *stmt);

    /// \copydoc StmtVisitor::visit
    virtual bool visit(Assign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(PhiAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(ImplicitAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    virtual bool visit(BoolAssign *stmt) override;
};
