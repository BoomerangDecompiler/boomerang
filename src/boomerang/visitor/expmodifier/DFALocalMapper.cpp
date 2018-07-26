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


#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/type/VoidType.h"


DfaLocalMapper::DfaLocalMapper(UserProc *proc)
    : m_proc(proc)
    , m_prog(proc->getProg())
    , m_sig(proc->getSignature())
{
}


bool DfaLocalMapper::processExp(const SharedExp& exp)
{
    if (m_proc->isLocalOrParamPattern(exp)) { // Check if this is an appropriate pattern for local variables
        const int spIndex = Util::getStackRegisterIndex(m_prog);
        if (m_sig->isStackLocal(spIndex, exp)) {
            change = true;                  // We've made a mapping
            // We have probably not even run TA yet, so doing a full descendtype here would be silly
            // Note also that void is compatible with all types, so the symbol effectively covers all types
            m_proc->getSymbolExp(exp, VoidType::get(), true);
        }

        return false; // set recur false: Don't dig inside m[x] to make m[a[m[x]]] !
    }

    return true;
}


SharedExp DfaLocalMapper::preModify(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    visitChildren = true;

    // Need the 2nd test to ensure change set correctly
    if (exp->isMemOf() && (m_proc->findFirstSymbol(exp) == nullptr)) {
        visitChildren = processExp(exp);
    }

    return exp;
}


SharedExp DfaLocalMapper::preModify(const std::shared_ptr<Binary>& exp, bool& visitChildren)
{
#if 1
    // Check for sp -/+ K
    SharedExp memOf_e = Location::memOf(exp);

    if (m_proc->findFirstSymbol(memOf_e) != nullptr) {
        visitChildren = false; // Already done; don't recurse
        return exp;
    }
    else {
        visitChildren = processExp(memOf_e);              // Process m[this]

        if (!visitChildren) {                             // If made a change this visit,
            return Unary::get(opAddrOf, memOf_e); // change to a[m[this]]
        }
    }
#endif
    return exp;
}


SharedExp DfaLocalMapper::preModify(const std::shared_ptr<TypedExp>& exp, bool& visitChildren)
{
    // Assume it's already been done correctly, so don't recurse into this
    visitChildren = false;
    return exp;
}
