#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpModifier.h"

#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/ssl/exp/Unary.h"


SharedExp ExpModifier::preModify(const std::shared_ptr<Unary> &exp, bool &visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preModify(const std::shared_ptr<Binary> &exp, bool &visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preModify(const std::shared_ptr<Ternary> &exp, bool &visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preModify(const std::shared_ptr<TypedExp> &exp, bool &visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preModify(const std::shared_ptr<RefExp> &exp, bool &visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preModify(const std::shared_ptr<Location> &exp, bool &visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<Unary> &exp)
{
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<Binary> &exp)
{
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<Ternary> &exp)
{
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<TypedExp> &exp)
{
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<RefExp> &exp)
{
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<Location> &exp)
{
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<Const> &exp)
{
    return exp;
}


SharedExp ExpModifier::postModify(const std::shared_ptr<Terminal> &exp)
{
    return exp;
}
