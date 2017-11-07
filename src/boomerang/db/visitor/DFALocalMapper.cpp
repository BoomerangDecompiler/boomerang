#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DFALocalMapper.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/type/type/VoidType.h"


DfaLocalMapper::DfaLocalMapper(UserProc *proc)
    : m_proc(proc)
{
    m_sig  = m_proc->getSignature();
    m_prog = m_proc->getProg();
    change = false;
}


bool DfaLocalMapper::processExp(const SharedExp& exp)
{
    if (m_proc->isLocalOrParamPattern(exp)) { // Check if this is an appropriate pattern for local variables
        if (m_sig->isStackLocal(m_prog, exp)) {
            change = true;                  // We've made a mapping
            // We have probably not even run TA yet, so doing a full descendtype here would be silly
            // Note also that void is compatible with all types, so the symbol effectively covers all types
            m_proc->getSymbolExp(exp, VoidType::get(), true);
        }

        return false; // set recur false: Don't dig inside m[x] to make m[a[m[x]]] !
    }

    return true;
}


SharedExp DfaLocalMapper::preVisit(const std::shared_ptr<Location>& e, bool& recur)
{
    recur = true;

    if (e->isMemOf() && (m_proc->findFirstSymbol(e) == nullptr)) { // Need the 2nd test to ensure change set correctly
        recur = processExp(e);
    }

    return e;
}


SharedExp DfaLocalMapper::preVisit(const std::shared_ptr<Binary>& exp, bool& recur)
{
#if 1
    // Check for sp -/+ K
    SharedExp memOf_e = Location::memOf(exp);

    if (m_proc->findFirstSymbol(memOf_e) != nullptr) {
        recur = false; // Already done; don't recurse
        return exp;
    }
    else {
        recur = processExp(memOf_e);              // Process m[this]

        if (!recur) {                             // If made a change this visit,
            return Unary::get(opAddrOf, memOf_e); // change to a[m[this]]
        }
    }
#endif
    return exp;
}


SharedExp DfaLocalMapper::preVisit(const std::shared_ptr<TypedExp>& e, bool& recur)
{
    // Assume it's already been done correctly, so don't recurse into this
    recur = false;
    return e;
}
