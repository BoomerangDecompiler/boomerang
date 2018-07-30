#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ConstGlobalConverter.h"


#include "boomerang/db/Global.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/ssl/type/ArrayType.h"


ConstGlobalConverter::ConstGlobalConverter(Prog* prog)
    : m_prog(prog)
{
}


SharedExp ConstGlobalConverter::preModify(const std::shared_ptr<RefExp>& exp, bool& visitChildren)
{
    Statement *def = exp->getDef();

    if (!def || def->isImplicit()) {
        SharedExp base = exp->getSubExp1();
        SharedExp addr = base->isMemOf() ? base->getSubExp1() : nullptr;

        if (base->isMemOf() && addr && addr->isIntConst()) {
            // We have a m[K]{-}
            Address K     = addr->access<Const>()->getAddr();
            int     value = m_prog->readNative4(K);
            visitChildren = false;
            return Const::get(value);
        }
        else if (base->isGlobal()) {
            // We have a glo{-}
            QString gname    = base->access<Const, 1>()->getStr();
            Address gloValue = m_prog->getGlobalAddrByName(gname);
            int     value    = m_prog->readNative4(gloValue);
            visitChildren = false;
            return Const::get(value);
        }
        else if (base->isArrayIndex()) {
            SharedExp idx = base->getSubExp2();
            SharedExp glo = base->getSubExp1();

            if (idx && idx->isIntConst() && glo && glo->isGlobal()) {
                // We have a glo[K]{-}
                int        K        = idx->access<Const>()->getInt();
                QString    gname    = glo->access<Const, 1>()->getStr();
                Address    gloValue = m_prog->getGlobalAddrByName(gname);
                SharedType gloType  = m_prog->getGlobalByName(gname)->getType();

                assert(gloType->isArray());
                SharedType componentType = gloType->as<ArrayType>()->getBaseType();
                int        value         = m_prog->readNative4(gloValue + K * (componentType->getSize() / 8));
                visitChildren = false;
                return Const::get(value);
            }
        }
    }

    visitChildren = true;
    return exp;
}

