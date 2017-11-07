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


StmtImplicitConverter::StmtImplicitConverter(ImplicitConverter* ic, Cfg* cfg)
    : StmtModifier(ic, false)  // False to not ignore collectors (want to make sure that
    , m_cfg(cfg)               //  collectors have valid expressions so you can ascendType)
{
}


void StmtImplicitConverter::visit(PhiAssign *s, bool& recur)
{
    // The LHS could be a m[x] where x has a null subscript; must do first
    s->setLeft(s->getLeft()->accept(m_mod));

    for (auto& v : *s) {
        assert(v.second.e != nullptr);

        if (v.second.getDef() == nullptr) {
            v.second.setDef(m_cfg->findImplicitAssign(v.second.e));
        }
    }

    recur = false; // Already done LHS
}
