#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SimpExpModifier.h"


#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/FlagDef.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/exp/Unary.h"


SimpExpModifier::SimpExpModifier()
{
    m_mask      = 1;
    m_unchanged = static_cast<unsigned int>(-1);
}


SharedExp SimpExpModifier::preModify(const std::shared_ptr<Unary>& exp, bool& visitChildren)
{
    visitChildren    = true;
    m_mask <<= 1;
    return exp;
}


SharedExp SimpExpModifier::preModify(const std::shared_ptr<Binary>& exp, bool& visitChildren)
{
    visitChildren    = true;
    m_mask <<= 1;
    return exp;
}


SharedExp SimpExpModifier::preModify(const std::shared_ptr<Ternary>& exp, bool& visitChildren)
{
    visitChildren    = true;
    m_mask <<= 1;
    return exp;
}


SharedExp SimpExpModifier::preModify(const std::shared_ptr<TypedExp>& exp, bool& visitChildren)
{
    visitChildren    = true;
    m_mask <<= 1;
    return exp;
}


SharedExp SimpExpModifier::preModify(const std::shared_ptr<FlagDef>& exp, bool& visitChildren)
{
    visitChildren    = true;
    m_mask <<= 1;
    return exp;
}


SharedExp SimpExpModifier::preModify(const std::shared_ptr<RefExp>& exp, bool& visitChildren)
{
    visitChildren    = true;
    m_mask <<= 1;
    return exp;
}


SharedExp SimpExpModifier::preModify(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    visitChildren    = true;
    m_mask <<= 1;
    return exp;
}


SharedExp SimpExpModifier::modify(const std::shared_ptr<Const>& exp)
{
    m_mask <<= 1;
    m_mask >>= 1;
    return exp;
}


SharedExp SimpExpModifier::modify(const std::shared_ptr<Terminal>& exp)
{
    m_mask <<= 1;
    m_mask >>= 1;
    return exp;
}


SharedExp SimpExpModifier::postModify(const std::shared_ptr<Location>& exp)
{
    SharedExp ret = exp;

    if (!(m_unchanged & m_mask)) {
        ret = exp->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postModify(const std::shared_ptr<RefExp>& exp)
{
    SharedExp ret = exp;

    if (!(m_unchanged & m_mask)) {
        ret = exp->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postModify(const std::shared_ptr<Unary>& exp)
{
    SharedExp ret = exp;

    if (!(m_unchanged & m_mask)) {
        ret = exp->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postModify(const std::shared_ptr<Binary>& exp)
{
    SharedExp ret = exp;

    if (!(m_unchanged & m_mask)) {
        ret = exp->simplifyArith()->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postModify(const std::shared_ptr<Ternary>& exp)
{
    SharedExp ret = exp;

    if (!(m_unchanged & m_mask)) {
        ret = exp->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postModify(const std::shared_ptr<TypedExp>& exp)
{
    SharedExp ret = exp;

    if (!(m_unchanged & m_mask)) {
        ret = exp->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postModify(const std::shared_ptr<FlagDef>& exp)
{
    SharedExp ret = exp;

    if (!(m_unchanged & m_mask)) {
        ret = exp->simplify();
    }

    m_mask >>= 1;
    return ret;
}
