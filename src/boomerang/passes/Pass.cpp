#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Pass.h"


#include <cassert>


IPass::IPass(const QString& name, PassID type)
    : m_name(name)
    , m_type(type)
{
    assert(!name.isEmpty());
}
