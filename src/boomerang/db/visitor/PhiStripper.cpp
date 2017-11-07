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


#include "boomerang/db/statements/PhiAssign.h"


void PhiStripper::visit(PhiAssign * /*stmt*/, bool& recur)
{
    m_del = true;
    recur = true;
}
