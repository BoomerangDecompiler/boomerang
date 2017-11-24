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


#include "boomerang/db/visitor/StmtExpVisitor.h"

class ExpRegMapper;
class Assignment;


/**
 *
 */
class StmtRegMapper : public StmtExpVisitor
{
public:
    StmtRegMapper(ExpRegMapper *erm);
    virtual ~StmtRegMapper() = default;

public:
    virtual bool common(Assignment *stmt, bool& visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(Assign *stmt, bool& visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(PhiAssign *stmt, bool& visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(ImplicitAssign *stmt, bool& visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(BoolAssign *stmt, bool& visitChildren) override;
};

