#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ImplicitConverter.h"


#include "boomerang/db/CFG.h"
#include "boomerang/ssl/exp/RefExp.h"


ImplicitConverter::ImplicitConverter(Cfg* cfg)
    : m_cfg(cfg)
{
}


SharedExp ImplicitConverter::postModify(const std::shared_ptr<RefExp>& exp)
{
    if (exp->getDef() == nullptr) {
        exp->setDef(m_cfg->findOrCreateImplicitAssign(exp->getSubExp1()));
    }

    return exp;
}
