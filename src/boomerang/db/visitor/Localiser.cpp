#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Localiser.h"


#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/statements/CallStatement.h"


Localiser::Localiser(CallStatement* c)
    : call(c)
{
}


SharedExp Localiser::preVisit(const std::shared_ptr<RefExp>& e, bool& recur)
{
    recur    = false; // Don't recurse into already subscripted variables
    m_mask <<= 1;
    return e;
}


SharedExp Localiser::preVisit(const std::shared_ptr<Location>& e, bool& recur)
{
    recur    = true;
    m_mask <<= 1;
    return e;
}


SharedExp Localiser::postVisit(const std::shared_ptr<Location>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    SharedExp r = call->findDefFor(ret);

    if (r) {
        ret = r->clone();

        ret          = ret->bypass();
        m_unchanged &= ~m_mask;
        m_mod        = true;
    }
    else {
        ret = RefExp::get(ret, nullptr); // No definition reaches, so subscript with {-}
    }

    return ret;
}


SharedExp Localiser::postVisit(const std::shared_ptr<Terminal>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    SharedExp r = call->findDefFor(ret);

    if (r) {
        ret          = r->clone()->bypass();
        m_unchanged &= ~m_mask;
        m_mod        = true;
    }
    else {
        ret = RefExp::get(ret, nullptr); // No definition reaches, so subscript with {-}
    }

    return ret;
}
