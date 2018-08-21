#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpConstCaster.h"

#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/ssl/type/Type.h"


ExpConstCaster::ExpConstCaster(int num, SharedType ty)
    : m_num(num)
    , m_ty(ty)
    , m_changed(false)
{
}


SharedExp ExpConstCaster::postModify(const std::shared_ptr<Const> &exp)
{
    if (exp->getConscript() == m_num) {
        m_changed = true;
        return std::make_shared<TypedExp>(m_ty, exp);
    }

    return exp;
}
