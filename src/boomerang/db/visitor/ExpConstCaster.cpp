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


#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/type/type/Type.h"


ExpConstCaster::ExpConstCaster(int _num, SharedType _ty)
    : m_num(_num)
    , m_ty(_ty)
    , m_changed(false)
{
}


SharedExp ExpConstCaster::preVisit(const std::shared_ptr<Const>& c)
{
    if (c->getConscript() == m_num) {
        m_changed = true;
        return std::make_shared<TypedExp>(m_ty, c);
    }

    return c;
}
