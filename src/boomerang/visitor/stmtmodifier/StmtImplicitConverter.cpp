#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtImplicitConverter.h"

#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/visitor/expmodifier/ImplicitConverter.h"


StmtImplicitConverter::StmtImplicitConverter(ImplicitConverter *ic, ProcCFG *cfg)
    : StmtModifier(ic, false) // False to not ignore collectors (want to make sure that
    , m_cfg(cfg)              //  collectors have valid expressions so you can ascendType)
{
}


void StmtImplicitConverter::visit(PhiAssign *stmt, bool &visitChildren)
{
    // The LHS could be a m[x] where x has a null subscript; must do first
    stmt->setLeft(stmt->getLeft()->acceptModifier(m_mod));

    for (const std::shared_ptr<RefExp> &exp : *stmt) {
        assert(exp->getSubExp1() != nullptr);

        if (exp->getDef() == nullptr) {
            exp->setDef(m_cfg->findOrCreateImplicitAssign(exp->getSubExp1()));
        }
    }

    visitChildren = false; // Already done LHS
}
