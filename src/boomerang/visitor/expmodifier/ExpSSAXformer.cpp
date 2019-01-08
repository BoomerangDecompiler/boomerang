#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpSSAXformer.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/log/Log.h"

ExpSSAXformer::ExpSSAXformer(UserProc *p)
    : m_proc(p)
{
}


SharedExp ExpSSAXformer::postModify(const std::shared_ptr<RefExp> &exp)
{
    QString sym = m_proc->lookupSymFromRefAny(exp);

    if (!sym.isEmpty()) {
        return Location::local(sym, m_proc);
    }

    // We should not get here: all locations should be replaced with Locals or Parameters
    if (m_proc->getProg()->getProject()->getSettings()->verboseOutput) {
        LOG_ERROR("Could not find local or parameter for %1!!", exp);
    }

    return exp->getSubExp1(); // At least strip off the subscript
}
