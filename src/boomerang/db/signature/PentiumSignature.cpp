#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PentiumSignature.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/util/log/Log.h"


namespace CallingConvention::StdC
{
PentiumSignature::PentiumSignature(const QString &name)
    : Signature(name)
{
    Signature::addReturn(Location::regOf(REG_PENT_ESP));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                 Location::regOf(REG_PENT_ESP), nullptr);
}


PentiumSignature::PentiumSignature(Signature &old)
    : Signature(old)
{
}


std::shared_ptr<Signature> PentiumSignature::clone() const
{
    PentiumSignature *n = new PentiumSignature(m_name);

    Util::clone(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    Util::clone(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;
    n->m_unknown       = m_unknown;

    return std::shared_ptr<Signature>(n);
}


bool PentiumSignature::operator==(const Signature &other) const
{
    return Signature::operator==(other);
}


bool PentiumSignature::qualified(UserProc *p, Signature & /*candidate*/)
{
    if (p->getProg()->getMachine() != Machine::PENTIUM) {
        return false;
    }

    LOG_VERBOSE2("Consider promotion to stdc pentium signature for %1", p->getName());

#if 1
    LOG_VERBOSE2("Promotion qualified: always true");
    return true; // For now, always pass
#else
    bool gotcorrectret1 = false;
    bool gotcorrectret2 = false;
    StatementList internal;
    // p->getInternalStatements(internal);
    internal.append(*p->getCFG()->getReachExit());
    StatementList::iterator it;

    for (Statement *s : internal) {
        Assign *e = dynamic_cast<Assign *>(s);

        if (e == nullptr) {
            continue;
        }

        if (e->getLeft()->isPC()) {
            if (e->getRight()->isMemOf() && e->getRight()->getSubExp1()->isRegN(REG_PENT_ESP)) {
                LOG_VERBOSE("Got pc = m[r[28]]");
                gotcorrectret1 = true;
            }
        }
        else if (e->getLeft()->isRegOfConst() &&
                 (e->getLeft()->getSubExp1()->access<Const>()->getInt() == REG_PENT_ESP)) {
            if ((e->getRight()->getOper() == opPlus) &&
                e->getRight()->getSubExp1()->isRegN(REG_PENT_ESP) &&
                e->getRight()->getSubExp2()->isIntConst() &&
                (e->getRight()->getSubExp2()->access<Const>()->getInt() == 4)) {
                LOG_VERBOSE("Got r[28] = r[28] + 4");
                gotcorrectret2 = true;
            }
        }
    }

    LOG_VERBOSE("Promotion: %1", gotcorrectret1 && gotcorrectret2);
    return gotcorrectret1 && gotcorrectret2;
#endif
}


RegNum PentiumSignature::getStackRegister() const
{
    return REG_PENT_ESP;
}


void PentiumSignature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        if (type->isFloat()) {
            e = Location::regOf(REG_PENT_ST0);
        }
        else {
            e = Location::regOf(REG_PENT_EAX);
        }
    }

    Signature::addReturn(type, e);
}


void PentiumSignature::addParameter(const QString &name, const SharedExp &e, SharedType type,
                                    const QString &boundMax)
{
    Signature::addParameter(name, e ? e : getArgumentExp(m_params.size()), type, boundMax);
}


SharedExp PentiumSignature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    SharedExp esp = Location::regOf(REG_PENT_ESP);

    if ((m_params.size() != 0) && (*m_params[0]->getExp() == *esp)) {
        n--;
    }

    return Location::memOf(Binary::get(opPlus, esp, Const::get((n + 1) * 4)));
}


std::shared_ptr<Signature> PentiumSignature::promote(UserProc * /*p*/)
{
    // No promotions from here up, obvious idea would be c++ name mangling
    return shared_from_this();
}


SharedExp PentiumSignature::getProven(SharedExp left) const
{
    if (left->isRegOfConst()) {
        const int r = left->access<Const, 1>()->getInt();

        switch (r) {
        case REG_PENT_ESP:                                                            // esp
            return Binary::get(opPlus, Location::regOf(REG_PENT_ESP), Const::get(4)); // esp+4

        case REG_PENT_BX:
        case REG_PENT_BP:
        case REG_PENT_SI:
        case REG_PENT_DI:
        case REG_PENT_BL:
        case REG_PENT_BH:
        case REG_PENT_EBX:
        case REG_PENT_EBP:
        case REG_PENT_ESI:
        case REG_PENT_EDI: return Location::regOf(r);
        }
    }

    return nullptr;
}


bool PentiumSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfConst()) {
        switch (e->access<Const, 1>()->getInt()) {
        case REG_PENT_EBP: // ebp
        case REG_PENT_EBX: // ebx
        case REG_PENT_ESI: // esi
        case REG_PENT_EDI: // edi
        case REG_PENT_BX:  // bx
        case REG_PENT_BP:  // bp
        case REG_PENT_SI:  // si
        case REG_PENT_DI:  // di
        case REG_PENT_BL:  // bl
        case REG_PENT_BH:  // bh
            return true;

        default: return false;
        }
    }

    return false;
}


void PentiumSignature::getLibraryDefines(StatementList &defs)
{
    if (defs.size() > 0) {
        // Do only once
        return;
    }

    auto r24      = Location::regOf(REG_PENT_EAX); // eax
    SharedType ty = SizeType::get(32);

    if (m_returns.size() > 1) { // Ugh - note the stack pointer is the first return still
        ty = m_returns[1]->getType();
    }

    defs.append(new ImplicitAssign(ty, r24));                       // eax
    defs.append(new ImplicitAssign(Location::regOf(REG_PENT_ECX))); // ecx
    defs.append(new ImplicitAssign(Location::regOf(REG_PENT_EDX))); // edx
    defs.append(new ImplicitAssign(Location::regOf(REG_PENT_ESP))); // esp
}


bool PentiumSignature::returnCompare(const Assignment &a, const Assignment &b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();

    // Eax is the preferred return location
    if (la->isRegN(REG_PENT_EAX)) {
        return true; // r24 is less than anything
    }
    else if (lb->isRegN(REG_PENT_EAX)) {
        return false; // Nothing is less than r24
    }

    // Next best is floating point %st
    if (la->isRegN(REG_PENT_ST0)) {
        return true; // r30 is less than anything that's left
    }
    else if (lb->isRegN(REG_PENT_ST0)) {
        return false; // Nothing left is less than r30
    }

    // Else don't care about the order
    return *la < *lb;
}


bool CallingConvention::StdC::PentiumSignature::argumentCompare(const Assignment &a,
                                                                const Assignment &b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();
    int ma            = Util::getStackOffset(la, REG_PENT_ESP);
    int mb            = Util::getStackOffset(lb, REG_PENT_ESP);

    if (ma && mb) {
        return ma < mb;
    }

    if (ma && !mb) {
        return true; // m[sp-K] is less than anything else
    }

    if (mb && !ma) {
        return false; // Nothing else is less than m[sp-K]
    }

    // Else don't care about the order
    return *la < *lb;
}

}
