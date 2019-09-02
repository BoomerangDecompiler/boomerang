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


#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"



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
    virtual bool common(const std::shared_ptr<Assignment> &stmt, bool &visitChildren);

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren) override;

    /// \copydoc StmtExpVisitor::visit
    virtual bool visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren) override;
};
