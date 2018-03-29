#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MIPSSignature.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/exp/Terminal.h"


namespace CallingConvention
{
namespace StdC
{

MIPSSignature::MIPSSignature(const QString& _name)
    : Signature(_name)
{
    Signature::addReturn(Location::regOf(2));
}


std::shared_ptr<Signature> MIPSSignature::clone() const
{
    MIPSSignature *n = new MIPSSignature(m_name);

    Util::clone(m_params, n->m_params);
    Util::clone(m_returns, n->m_returns);

    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;

    if (m_preferredReturn) {
        n->m_preferredReturn = m_preferredReturn->clone();
    }
    else {
        n->m_preferredReturn = nullptr;
    }

    n->m_preferredParams = m_preferredParams;
    n->m_unknown         = m_unknown;
    return std::shared_ptr<Signature>(n);
}


bool MIPSSignature::qualified(UserProc *p, Signature& /*candidate*/)
{
    LOG_VERBOSE("Consider promotion to stdc MIPS signature for %1", p->getName());

    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::MIPS) {
        return false;
    }

    LOG_VERBOSE("Promoted to StdC::MIPSSignature (always qualifies)");

    return true;
}


void MIPSSignature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        if (type->isInteger() || type->isPointer()) {
            e = Location::regOf(2); // register $2
        }
        else if (type->isFloat()) {
            e = Location::regOf(32); // register $f0
        }
        else {
            e = Location::regOf(2); // register $2
        }
    }

    Signature::addReturn(type, e);
}


SharedExp MIPSSignature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    SharedExp e;

    if (n >= 4) {
        // MIPS abi - pass the 4th and subsequent parameters at m[%sp+home_locations],
        // theo sp +0 .. home_locations contains a 'shadow' set of locations for first parameters
        // m[%esp+home_locations], etc.
        //
        e = Location::memOf(Binary::get(opPlus,
                                        Location::regOf(29), // %o6 == %sp
                                        Const::get(4 * 4 + (n - 4) * 4)));
    }
    else {
        e = Location::regOf(static_cast<int>(8 + n));
    }

    return e;
}


void MIPSSignature::addParameter(SharedType type, const QString& name, const SharedExp& e, const QString& boundMax)
{
    Signature::addParameter(type, name, e ? e : getArgumentExp(m_params.size()), boundMax);
}


SharedExp MIPSSignature::getProven(SharedExp left) const
{
    if (left->isRegOfK()) {
        int r = left->access<Const, 1>()->getInt();

        if (r == getStackRegister()) {
            return left;
        }
    }

    return nullptr;
}


bool MIPSSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfK()) {
        int r = e->access<Const, 1>()->getInt();
        return r == getStackRegister();
    }

    return false;
}


void MIPSSignature::getLibraryDefines(StatementList& defs)
{
    if (defs.size() > 0) {
        return; // Do only once
    }

    for (int r = 16; r <= 23; ++r) {
        defs.append(new ImplicitAssign(Location::regOf(r))); // Registers 16-23 are volatile (caller save)
    }

    defs.append(new ImplicitAssign(Location::regOf(30)));
}

}
}
