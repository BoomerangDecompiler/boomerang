#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpRegMapper.h"


#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/proc/UserProc.h"


ExpRegMapper::ExpRegMapper(UserProc *p)
    : m_proc(p)
{
    m_prog = m_proc->getProg();
}


bool ExpRegMapper::preVisit(const std::shared_ptr<RefExp>& e, bool& visitChildren)
{
    SharedExp base = e->getSubExp1();

    if (base->isRegOf() || m_proc->isLocalOrParamPattern(base)) { // Don't convert if e.g. a global
        m_proc->ensureExpIsMappedToLocal(e);
    }

    visitChildren = false; // Don't examine the r[] inside
    return true;
}
