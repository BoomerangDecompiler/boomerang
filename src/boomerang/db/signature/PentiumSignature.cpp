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
    Signature::addReturn(Location::regOf(28));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                 Location::regOf(28), nullptr);
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
    n->m_unknown         = m_unknown;

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
    StmtListIter it;

    for (Statement *s = internal.getFirst(it); s; s = internal.getNext(it)) {
        Assign *e = dynamic_cast<Assign *>(s);

        if (e == nullptr) {
            continue;
        }

        if (e->getLeft()->getOper() == opPC) {
            if (e->getRight()->isMemOf() && e->getRight()->getSubExp1()->isRegOfN(28)) {
                LOG_VERBOSE("Got pc = m[r[28]]");
                gotcorrectret1 = true;
            }
        }
        else if (e->getLeft()->isRegOfK() && (((Const *)e->getLeft()->getSubExp1())->getInt() == 28)) {
            if ((e->getRight()->getOper() == opPlus) && e->getRight()->getSubExp1()->isRegOfN(28) &&
                e->getRight()->getSubExp2()->isIntConst() && (((Const *)e->getRight()->getSubExp2())->getInt() == 4)) {
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
            e = Location::regOf(32);
        }
        else {
            e = Location::regOf(24);
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

    SharedExp esp = Location::regOf(28);

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
    if (left->isRegOfK()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r)
        {
        case 28:                                                            // esp
            return Binary::get(opPlus, Location::regOf(28), Const::get(4)); // esp+4

        case 29:
        case 30:
        case 31:
        case 27: // ebp, esi, edi, ebx
            return Location::regOf(r);
        }
    }

    return nullptr;
}


bool PentiumSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfK()) {
        switch (e->access<Const, 1>()->getInt())
        {
        case 29: // ebp
        case 27: // ebx
        case 30: // esi
        case 31: // edi
        case 3:  // bx
        case 5:  // bp
        case 6:  // si
        case 7:  // di
        case 11: // bl
        case 15: // bh
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

    auto       r24 = Location::regOf(24); // eax
    SharedType ty  = SizeType::get(32);

    if (m_returns.size() > 1) {                  // Ugh - note the stack pointer is the first return still
        ty = m_returns[1]->getType();
    }

    defs.append(new ImplicitAssign(ty, r24));             // eax
    defs.append(new ImplicitAssign(Location::regOf(25))); // ecx
    defs.append(new ImplicitAssign(Location::regOf(26))); // edx
    defs.append(new ImplicitAssign(Location::regOf(28))); // esp
}


bool PentiumSignature::returnCompare(const Assignment& a, const Assignment& b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();

    // Eax is the preferred return location
    if (la->isRegN(24)) {
        return true; // r24 is less than anything
    }

    if (lb->isRegN(24)) {
        return false; // Nothing is less than r24
    }

    // Next best is r30 (floating point %st)
    if (la->isRegN(30)) {
        return true; // r30 is less than anything that's left
    }

    if (lb->isRegN(30)) {
        return false; // Nothing left is less than r30
    }

    // Else don't care about the order
    return *la < *lb;
}


bool CallingConvention::StdC::PentiumSignature::argumentCompare(const Assignment& a, const Assignment& b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();
    int       ma = Util::getStackOffset(la, 28);
    int       mb = Util::getStackOffset(lb, 28);

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
