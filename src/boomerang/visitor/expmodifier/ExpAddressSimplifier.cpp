#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpAddressSimplifier.h"

#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Unary.h"


SharedExp ExpAddressSimplifier::preModify(const std::shared_ptr<Unary> &exp, bool &)
{
    if (exp->isAddrOf()) {
        if (exp->getSubExp1()->isMemOf()) {
            m_modified = true;
            return exp->getSubExp1()->getSubExp1();
        }
    }

    return exp->shared_from_this();
}


SharedExp ExpAddressSimplifier::preModify(const std::shared_ptr<Location> &exp, bool &)
{
    if (exp->isMemOf() && exp->getSubExp1()->isAddrOf()) {
        m_modified = true;
        return exp->getSubExp1()->getSubExp1();
    }

    return exp->shared_from_this();
}
