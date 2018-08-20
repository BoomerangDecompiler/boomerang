#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PhiStripper.h"

#include "boomerang/ssl/statements/PhiAssign.h"


PhiStripper::PhiStripper(ExpModifier *em)
    : StmtModifier(em)
    , m_del(false)
{}


void PhiStripper::visit(PhiAssign * /*stmt*/, bool &visitChildren)
{
    m_del         = true;
    visitChildren = true;
}
