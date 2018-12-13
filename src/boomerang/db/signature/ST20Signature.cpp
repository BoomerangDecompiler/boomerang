#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ST20Signature.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/util/log/Log.h"


namespace CallingConvention::StdC
{
ST20Signature::ST20Signature(const QString &name)
    : Signature(name)
{
    Signature::addReturn(Location::regOf(REG_ST20_C));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "sp",
    // Location::regOf(REG_ST20_SP), nullptr);
    // FIXME: Should also add m[sp+0] as an implicit parameter? Holds return address
}


ST20Signature::ST20Signature(Signature &old)
    : Signature(old)
{
}


std::shared_ptr<Signature> ST20Signature::clone() const
{
    ST20Signature *n = new ST20Signature(m_name);

    n->m_params        = m_params;
    n->m_returns       = m_returns;
    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;
    n->m_unknown       = m_unknown;

    return std::shared_ptr<Signature>(n);
}


bool ST20Signature::operator==(const Signature &other) const
{
    return Signature::operator==(other);
}


SharedExp ST20Signature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    // m[%sp+4], etc.
    SharedExp sp = Location::regOf(REG_ST20_SP);

    if ((m_params.size() != 0) && (*m_params[0]->getExp() == *sp)) {
        n--;
    }

    return Location::memOf(Binary::get(opPlus, sp, Const::get((n + 1) * 4)));
}


void ST20Signature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        e = Location::regOf(REG_ST20_A);
    }

    Signature::addReturn(type, e);
}


std::shared_ptr<Signature> ST20Signature::promote(UserProc * /*p*/)
{
    // No promotions from here up, obvious idea would be c++ name mangling
    return shared_from_this();
}


void ST20Signature::addParameter(const QString &name, const SharedExp &e, SharedType type,
                                 const QString &boundMax)
{
    Signature::addParameter(name, e ? e : getArgumentExp(m_params.size()), type, boundMax);
}


SharedExp ST20Signature::getProven(SharedExp left) const
{
    if (left->isRegOfConst()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r) {
        case REG_ST20_SP: return left;

        case REG_ST20_A:
        case REG_ST20_B:
        case REG_ST20_C:
            // Registers A, B, and C are callee save
            return Location::regOf(r);
        }
    }

    return nullptr;
}


bool ST20Signature::qualified(UserProc *p, Signature & /*candidate*/)
{
    if (p->getProg()->getMachine() != Machine::ST20) {
        return false;
    }

    LOG_VERBOSE2("Consider promotion to stdc st20 signature for %1", p->getName());

    return true;
}

}
