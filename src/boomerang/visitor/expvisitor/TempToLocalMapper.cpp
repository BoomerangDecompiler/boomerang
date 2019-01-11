#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TempToLocalMapper.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"


TempToLocalMapper::TempToLocalMapper(UserProc *proc)
    : m_proc(proc)
{
}


bool TempToLocalMapper::preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren)
{
    if (exp->isTemp()) {
        // We have a temp subexpression; get its name
        const QString tempName = exp->access<Const, 1>()->getStr();

        // This call will do the mapping from the temp to a new local:
        m_proc->getSymbolExp(exp, getTempType(tempName), true);
    }

    visitChildren = false; // No need to examine the string
    return true;
}


SharedType TempToLocalMapper::getTempType(const QString &name)
{
    if (name.size() <= 3) {
        return IntegerType::get(32);
    }

    const QChar ctype = name[3];

    switch (ctype.toLatin1()) {
    // They are all int32, except for a few specials
    case 'f': return FloatType::get(32);
    case 'd': return FloatType::get(64);
    case 'F': return FloatType::get(80);
    case 'D': return FloatType::get(128);
    case 'l': return IntegerType::get(64);
    case 'h': return IntegerType::get(16);
    case 'b': return IntegerType::get(8);
    default: return IntegerType::get(32);
    }
}
