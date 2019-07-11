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
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/ssl/type/ArrayType.h"


ConstGlobalConverter::ConstGlobalConverter(Prog *prog)
    : m_prog(prog)
{
}


SharedExp ConstGlobalConverter::preModify(const std::shared_ptr<RefExp> &exp, bool &visitChildren)
{
    Statement *def     = exp->getDef();
    BinaryImage *image = m_prog->getBinaryFile()->getImage();

    if (def && !def->isImplicit()) {
        visitChildren = true;
        return exp;
    }

    SharedExp base = exp->getSubExp1();
    SharedExp addr = base->isMemOf() ? base->getSubExp1() : nullptr;

    if (base->isMemOf() && addr && addr->isIntConst()) {
        // We have a m[K]{-}
        DWord value     = 0;
        const Address K = addr->access<Const>()->getAddr();
        if (image->readNative4(K, value)) {
            visitChildren = false;
            return Const::get((int)value);
        }
    }
    else if (base->isGlobal()) {
        // We have a glo{-}
        const QString gname   = base->access<Const, 1>()->getStr();
        const Address gloAddr = m_prog->getGlobalAddrByName(gname);
        DWord value           = 0;

        if (gloAddr != Address::INVALID && image->readNative4(gloAddr, value)) {
            visitChildren = false;
            return Const::get((int)value);
        }
    }
    else if (base->isArrayIndex()) {
        SharedExp idx = base->getSubExp2();
        SharedExp glo = base->getSubExp1();

        if (idx && idx->isIntConst() && glo && glo->isGlobal()) {
            // We have a glo[K]{-}
            const int K            = idx->access<Const>()->getInt();
            const QString gname    = glo->access<Const, 1>()->getStr();
            const Address gloValue = m_prog->getGlobalAddrByName(gname);

            if (gloValue != Address::INVALID) {
                DWord value             = 0;
                SharedConstType gloType = m_prog->getGlobalByName(gname)->getType();

                assert(gloType->isArray());
                SharedConstType componentType = gloType->as<ArrayType>()->getBaseType();

                if (image->readNative4(gloValue + K * componentType->getSize() / 8, value)) {
                    visitChildren = false;
                    return Const::get((int)value);
                }
            }
        }
    }

    visitChildren = true;
    return exp;
}
