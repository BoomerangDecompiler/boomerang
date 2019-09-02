#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtRegMapper.h"

#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/visitor/expvisitor/ExpRegMapper.h"


StmtRegMapper::StmtRegMapper(ExpRegMapper *erm)
    : StmtExpVisitor(erm)
{
}


bool StmtRegMapper::common(const std::shared_ptr<Assignment> &stmt, bool &visitChildren)
{
    // In case lhs is a reg or m[reg] such that reg is otherwise unused
    SharedExp lhs = stmt->getLeft();
    auto re       = RefExp::get(lhs, stmt);

    re->acceptVisitor(ev);
    visitChildren = true;
    return true;
}


bool StmtRegMapper::visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren)
{
    return common(stmt, visitChildren);
}


bool StmtRegMapper::visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren)
{
    return common(stmt, visitChildren);
}


bool StmtRegMapper::visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren)
{
    return common(stmt, visitChildren);
}


bool StmtRegMapper::visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren)
{
    return common(stmt, visitChildren);
}
