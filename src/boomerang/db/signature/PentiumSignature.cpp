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


#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/statements/ImplicitAssign.h"


namespace CallingConvention
{
namespace StdC
{

PentiumSignature::PentiumSignature(const QString& name)
    : Signature(name)
{
    Signature::addReturn(Location::regOf(PENT_REG_ESP));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                 Location::regOf(PENT_REG_ESP), nullptr);
}


PentiumSignature::PentiumSignature(Signature& old)
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


bool PentiumSignature::operator==(const Signature& other) const
{
    return Signature::operator==(other);
}


bool PentiumSignature::qualified(UserProc *p, Signature& /*candidate*/)
{
    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::PENTIUM) {
        return false;
    }

    LOG_VERBOSE2("Consider promotion to stdc pentium signature for %1", p->getName());

#if 1
    LOG_VERBOSE2("Promotion qualified: always true");
    return true; // For now, always pass
#else
    bool          gotcorrectret1 = false;
    bool          gotcorrectret2 = false;
    StatementList internal;
    // p->getInternalStatements(internal);
    internal.append(*p->getCFG()->getReachExit());
    StatementList::iterator it;

    for (Statement *s : internal) {
        Assign *e = dynamic_cast<Assign *>(s);

        if (e == nullptr) {
            continue;
        }

        if (e->getLeft()->getOper() == opPC) {
            if (e->getRight()->isMemOf() && e->getRight()->getSubExp1()->isRegN(PENT_REG_ESP)) {
                LOG_VERBOSE("Got pc = m[r[28]]");
                gotcorrectret1 = true;
            }
        }
        else if (e->getLeft()->isRegOfConst() && (e->getLeft()->getSubExp1()->access<Const>()->getInt() == PENT_REG_ESP)) {
            if ((e->getRight()->getOper() == opPlus) && e->getRight()->getSubExp1()->isRegN(PENT_REG_ESP) &&
                e->getRight()->getSubExp2()->isIntConst() && (e->getRight()->getSubExp2()->access<Const>()->getInt() == 4)) {
                LOG_VERBOSE("Got r[28] = r[28] + 4");
                gotcorrectret2 = true;
            }
        }
    }

    LOG_VERBOSE("Promotion: %1", gotcorrectret1 && gotcorrectret2);
    return gotcorrectret1 && gotcorrectret2;
#endif
}


void PentiumSignature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        if (type->isFloat()) {
            e = Location::regOf(PENT_REG_ST0);
        }
        else {
            e = Location::regOf(PENT_REG_EAX);
        }
    }

    Signature::addReturn(type, e);
}


void PentiumSignature::addParameter(const QString& name, const SharedExp& e,
                                    SharedType type, const QString& boundMax)
{
    Signature::addParameter(name, e ? e : getArgumentExp(m_params.size()), type, boundMax);
}


SharedExp PentiumSignature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    SharedExp esp = Location::regOf(PENT_REG_ESP);

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
        int r = left->access<Const, 1>()->getInt();

        switch (r)
        {
        case PENT_REG_ESP:                                                            // esp
            return Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(4)); // esp+4

        case PENT_REG_EBP:
        case PENT_REG_ESI:
        case PENT_REG_EDI:
        case PENT_REG_EBX:
            return Location::regOf(r);
        }
    }

    return nullptr;
}


bool PentiumSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfConst()) {
        switch (e->access<Const, 1>()->getInt())
        {
        case PENT_REG_EBP: // ebp
        case PENT_REG_EBX: // ebx
        case PENT_REG_ESI: // esi
        case PENT_REG_EDI: // edi
        case PENT_REG_BX:  // bx
        case PENT_REG_BP:  // bp
        case PENT_REG_SI:  // si
        case PENT_REG_DI:  // di
        case PENT_REG_BL:  // bl
        case PENT_REG_BH: // bh
            return true;

        default:
            return false;
        }
    }

    return false;
}


void PentiumSignature::getLibraryDefines(StatementList& defs)
{
    if (defs.size() > 0) {
        // Do only once
        return;
    }

    auto       r24 = Location::regOf(PENT_REG_EAX); // eax
    SharedType ty  = SizeType::get(32);

    if (m_returns.size() > 1) {                  // Ugh - note the stack pointer is the first return still
        ty = m_returns[1]->getType();
    }

    defs.append(new ImplicitAssign(ty, r24));             // eax
    defs.append(new ImplicitAssign(Location::regOf(PENT_REG_ECX))); // ecx
    defs.append(new ImplicitAssign(Location::regOf(PENT_REG_EDX))); // edx
    defs.append(new ImplicitAssign(Location::regOf(PENT_REG_ESP))); // esp
}


bool PentiumSignature::returnCompare(const Assignment& a, const Assignment& b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();

    // Eax is the preferred return location
    if (la->isRegN(PENT_REG_EAX)) {
        return true; // r24 is less than anything
    }
    else if (lb->isRegN(PENT_REG_EAX)) {
        return false; // Nothing is less than r24
    }

    // Next best is floating point %st
    if (la->isRegN(PENT_REG_ST0)) {
        return true; // r30 is less than anything that's left
    }
    else if (lb->isRegN(PENT_REG_ST0)) {
        return false; // Nothing left is less than r30
    }

    // Else don't care about the order
    return *la < *lb;
}


bool CallingConvention::StdC::PentiumSignature::argumentCompare(const Assignment& a, const Assignment& b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();
    int       ma = Util::getStackOffset(la, PENT_REG_ESP);
    int       mb = Util::getStackOffset(lb, PENT_REG_ESP);

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
}
