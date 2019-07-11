#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SPARCSignature.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/util/log/Log.h"


namespace CallingConvention::StdC
{
SPARCSignature::SPARCSignature(const QString &name)
    : Signature(name)
{
    Signature::addReturn(Location::regOf(REG_SPARC_SP));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "sp",
    //                                Location::regOf(REG_SPARC_SP), nullptr);
}


SPARCSignature::SPARCSignature(Signature &old)
    : Signature(old)
{
}


bool SPARCSignature::operator==(const Signature &other) const
{
    return Signature::operator==(other);
}


std::shared_ptr<Signature> SPARCSignature::clone() const
{
    SPARCSignature *n = new SPARCSignature(m_name);

    Util::clone(m_params, n->m_params);
    Util::clone(m_returns, n->m_returns);

    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;

    n->m_unknown = m_unknown;
    return std::shared_ptr<Signature>(n);
}


void SPARCSignature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        e = Location::regOf(REG_SPARC_O0);
    }

    Signature::addReturn(type, e);
}


void SPARCSignature::addParameter(const QString &name, const SharedExp &e, SharedType type,
                                  const QString &boundMax)
{
    Signature::addParameter(name, e ? e : getArgumentExp(m_params.size()), type, boundMax);
}


SharedExp SPARCSignature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    SharedExp e;

    if (n >= 6) {
        // SPARCs pass the seventh and subsequent parameters at m[%sp+92],
        // m[%esp+96], etc.
        e = Location::memOf(Binary::get(opPlus,
                                        Location::regOf(REG_SPARC_SP), // %o6 == %sp
                                        Const::get(92 + (n - 6) * 4)));
    }
    else {
        e = Location::regOf(REG_SPARC_O0 + n);
    }

    return e;
}


std::shared_ptr<Signature> SPARCSignature::promote(UserProc * /*p*/)
{
    // no promotions from here up, obvious example would be name mangling
    return shared_from_this();
}


SharedExp SPARCSignature::getProven(SharedExp left) const
{
    if (left->isRegOfConst()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r) {
        // These registers are preserved in SPARC: i0-i7 (24-31), sp (14)
        case REG_SPARC_SP: // sp
        case REG_SPARC_I0:
        case REG_SPARC_I1:
        case REG_SPARC_I2:
        case REG_SPARC_I3: // i0-i3
        case REG_SPARC_I4:
        case REG_SPARC_I5:
        case REG_SPARC_I6:
        case REG_SPARC_I7: // i4-i7
            // NOTE: Registers %g2 to %g4 are NOT preserved in ordinary application (non library)
            // code
            return left;
        }
    }

    return nullptr;
}


bool SPARCSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfConst()) {
        int r = e->access<Const, 1>()->getInt();

        switch (r) {
        // These registers are preserved in SPARC: i0-i7 (24-31), sp (14)
        case REG_SPARC_SP: // sp
        case REG_SPARC_I0:
        case REG_SPARC_I1:
        case REG_SPARC_I2:
        case REG_SPARC_I3: // i0-i3
        case REG_SPARC_I4:
        case REG_SPARC_I5:
        case REG_SPARC_I6:
        case REG_SPARC_I7: // i4-i7
            // NOTE: Registers %g2 to %g4 are NOT preserved in ordinary application (non library)
            // code
            return true;

        default: return false;
        }
    }

    return false;
}


void SPARCSignature::getLibraryDefines(StatementList &defs)
{
    if (defs.size() > 0) {
        return; // Do only once
    }

    // o0-o7 (r8-r15) modified
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O0)));
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O1)));
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O2)));
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O3)));
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O4)));
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O5)));
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O6)));
    defs.append(new ImplicitAssign(Location::regOf(REG_SPARC_O7)));
}


SharedExp SPARCLibSignature::getProven(SharedExp left) const
{
    if (left->isRegOfConst()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r) {
        // These registers are preserved in SPARC: i0-i7 (24-31), sp (14)
        case REG_SPARC_SP:
        case REG_SPARC_I0:
        case REG_SPARC_I1:
        case REG_SPARC_I2:
        case REG_SPARC_I3:
        case REG_SPARC_I4:
        case REG_SPARC_I5:
        case REG_SPARC_I6:
        case REG_SPARC_I7:
        // Also the "application global registers" g2-g4 (2-4) (preserved
        // by library functions, but apparently don't have to be preserved
        // by application code)
        case REG_SPARC_G2:
        case REG_SPARC_G3:
        case REG_SPARC_G4: // g2-g4
            // The system global registers (g5-g7) are also preserved, but
            // should never be changed in an application anyway
            return left;
        }
    }

    return nullptr;
}

bool SPARCSignature::qualified(UserProc *p, Signature & /*candidate*/)
{
    LOG_VERBOSE2("Consider promotion to stdc SPARC signature for %1", p->getName());

    if (p->getProg()->getMachine() != Machine::SPARC) {
        return false;
    }

    LOG_VERBOSE2("Promoted to StdC::SPARCSignature");
    return true;
}


bool SPARCSignature::isAddrOfStackLocal(RegNum spIndex, const SharedConstExp &e) const
{
    if (!Signature::isAddrOfStackLocal(spIndex, e)) {
        return false;
    }

    // SPARC specific test: K must be < 92; else it is a parameter
    SharedConstExp simplified = e->clone()->simplify();
    if (!simplified->getSubExp2()) {
        // bare sp
        return true;
    }
    else if (!simplified->getSubExp2()->isIntConst()) {
        return false;
    }

    const int offsetFromSP = simplified->getSubExp2()->access<Const>()->getInt();
    return simplified->getOper() == opMinus && offsetFromSP < 92;
}

static Unary spPlus64(opMemOf, Binary::get(opPlus, Location::regOf(REG_SPARC_SP), Const::get(64)));

bool SPARCSignature::returnCompare(const Assignment &a, const Assignment &b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();

    // %o0 (r8) is the preferred return location
    if (la->isRegN(REG_SPARC_O0)) {
        return true; // r24 is less than anything
    }

    if (lb->isRegN(REG_SPARC_O0)) {
        return false; // Nothing is less than r24
    }

    // Next best is %f0 (r32)
    if (la->isRegN(REG_SPARC_F0)) {
        return true; // r32 is less than anything that's left
    }

    if (lb->isRegN(REG_SPARC_F0)) {
        return false; // Nothing left is less than r32
    }

    // Next best is %f0-1 (r64)
    if (la->isRegN(REG_SPARC_F0TO1)) {
        return true; // r64 is less than anything that's left
    }

    if (lb->isRegN(REG_SPARC_F0TO1)) {
        return false; // Nothing left is less than r64
    }

    // Next best is m[esp{-}+64]
    if (*la == spPlus64) {
        return true; // m[esp{-}+64] is less than anything that's left
    }

    if (*lb == spPlus64) {
        return false; // Nothing left is less than m[esp{-}+64]
    }

    // Else don't care about the order
    return *la < *lb;
}


bool SPARCSignature::argumentCompare(const Assignment &a, const Assignment &b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();
    // %o0-$o5 (r8-r13) are the preferred argument locations
    int ra = 0, rb = 0;

    if (la->isRegOf()) {
        int r = la->access<Const, 1>()->getInt();

        if ((r >= REG_SPARC_O0) && (r <= REG_SPARC_O5)) {
            ra = r;
        }
    }

    if (lb->isRegOf()) {
        int r = lb->access<Const, 1>()->getInt();

        if ((r >= REG_SPARC_O0) && (r <= REG_SPARC_O5)) {
            rb = r;
        }
    }

    if (ra && rb) {
        return ra < rb; // Both r8-r13: compare within this set
    }

    if (ra && (rb == 0)) {
        return true; // r8-r13 less than anything else
    }

    if (rb && (ra == 0)) {
        return false; // Nothing else is less than r8-r13
    }

    const int ma = Util::getStackOffset(la, REG_SPARC_FP);
    const int mb = Util::getStackOffset(lb, REG_SPARC_FP);

    if (ma && mb) {
        return ma < mb; // Both m[sp + K]: order by memory offset
    }

    if (ma && !mb) {
        return true; // m[sp+K] less than anything left
    }

    if (mb && !ma) {
        return false; // nothing left is less than m[sp+K]
    }

    return *la < *lb; // Else order arbitrarily
}

}
