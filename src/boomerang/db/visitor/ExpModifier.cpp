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


#include "boomerang/db/exp/Unary.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/exp/FlagDef.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypeVal.h"


SharedExp ExpModifier::preVisit(const std::shared_ptr<Unary>& exp, bool& visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<Binary>& exp, bool& visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<Ternary>& exp, bool& visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<TypedExp>& exp, bool& visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<FlagDef>& exp, bool& visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<RefExp>& exp, bool& visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    visitChildren = true;
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<Const>& exp)
{
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<Terminal>& exp)
{
    return exp;
}


SharedExp ExpModifier::preVisit(const std::shared_ptr<TypeVal>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<Unary>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<Binary>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<Ternary>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<TypedExp>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<FlagDef>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<RefExp>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<Location>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<Const>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<Terminal>& exp)
{
    return exp;
}


SharedExp ExpModifier::postVisit(const std::shared_ptr<TypeVal>& exp)
{
    return exp;
}
