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


#include "boomerang/db/CFG.h"
#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/visitor/ImplicitConverter.h"
#include "boomerang/db/exp/RefExp.h"


StmtImplicitConverter::StmtImplicitConverter(ImplicitConverter* ic, Cfg* cfg)
    : StmtModifier(ic, false)  // False to not ignore collectors (want to make sure that
    , m_cfg(cfg)               //  collectors have valid expressions so you can ascendType)
{
}


void StmtImplicitConverter::visit(PhiAssign *stmt, bool& visitChildren)
{
    // The LHS could be a m[x] where x has a null subscript; must do first
    stmt->setLeft(stmt->getLeft()->accept(m_mod));

    for (RefExp& exp : *stmt) {
        assert(exp.getSubExp1() != nullptr);

        if (exp.getDef() == nullptr) {
            exp.setDef(m_cfg->findImplicitAssign(exp.getSubExp1()));
        }
    }

    visitChildren = false; // Already done LHS
}
