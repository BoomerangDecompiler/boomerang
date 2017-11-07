#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpVisitor.h"


bool ExpVisitor::visit(const std::shared_ptr<Unary>&, bool& override)
{
    override = false;
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<Binary>&, bool& override)
{
    override = false;
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<Ternary>&, bool& override)
{
    override = false;
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<TypedExp>&, bool& override)
{
    override = false;
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<FlagDef>&, bool& override)
{
    override = false;
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<RefExp>&, bool& override)
{
    override = false;
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<Location>&, bool& override)
{
    override = false;
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<Const>&)
{
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<Terminal>&)
{
    return true;
}


bool ExpVisitor::visit(const std::shared_ptr<TypeVal>&)
{
    return true;
}

