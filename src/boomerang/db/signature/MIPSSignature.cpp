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


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/util/log/Log.h"


namespace CallingConvention
{
namespace StdC
{

MIPSSignature::MIPSSignature(const QString& _name)
    : Signature(_name)
{
    Signature::addReturn(Location::regOf(REG_MIPS_V0));
}


std::shared_ptr<Signature> MIPSSignature::clone() const
{
    MIPSSignature *n = new MIPSSignature(m_name);

    Util::clone(m_params, n->m_params);
    Util::clone(m_returns, n->m_returns);

    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;
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
            e = Location::regOf(REG_MIPS_V0); // register $2
        }
        else if (type->isFloat()) {
            e = Location::regOf(REG_MIPS_F0); // register $f0
        }
        else {
            e = Location::regOf(REG_MIPS_V0); // register $2
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
                                        Location::regOf(REG_MIPS_SP), // %o6 == %sp
                                        Const::get(4 * 4 + (n - 4) * 4)));
    }
    else {
        e = Location::regOf(REG_MIPS_T0 + n);
    }

    return e;
}


void MIPSSignature::addParameter(const QString& name, const SharedExp& e,
                                 SharedType type, const QString& boundMax)
{
    Signature::addParameter(name, e ? e : getArgumentExp(m_params.size()), type, boundMax);
}


SharedExp MIPSSignature::getProven(SharedExp left) const
{
    if (left->isRegOfConst()) {
        int r = left->access<Const, 1>()->getInt();

        if (r == getStackRegister()) {
            return left;
        }
    }

    return nullptr;
}


bool MIPSSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfConst()) {
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

    defs.append(new ImplicitAssign(Location::regOf(REG_MIPS_FP)));
}

}
}
