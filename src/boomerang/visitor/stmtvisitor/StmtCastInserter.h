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


#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"


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
    StmtCastInserter()          = default;
    virtual ~StmtCastInserter() = default;

public:
    bool common(const Assignment *stmt);

    /// \copydoc StmtVisitor::visit
    bool visit(const Assign *stmt) override;

    /// \copydoc StmtVisitor::visit
    bool visit(const PhiAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    bool visit(const ImplicitAssign *stmt) override;

    /// \copydoc StmtVisitor::visit
    bool visit(const BoolAssign *stmt) override;
};
