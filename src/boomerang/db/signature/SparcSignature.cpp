#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SparcSignature.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/exp/RefExp.h"


namespace CallingConvention
{
namespace StdC
{


SparcSignature::SparcSignature(const QString& name)
    : Signature(name)
{
    Signature::addReturn(Location::regOf(14));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "sp",
    //                                Location::regOf(14), nullptr);
}


SparcSignature::SparcSignature(Signature& old)
    : Signature(old)
{
}


bool SparcSignature::operator==(const Signature& other) const
{
    return Signature::operator==(other);
}


std::shared_ptr<Signature> SparcSignature::clone() const
{
    SparcSignature *n = new SparcSignature(m_name);

    Util::clone(m_params, n->m_params);
    Util::clone(m_returns, n->m_returns);

    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;

    n->m_unknown         = m_unknown;
    return std::shared_ptr<Signature>(n);
}


void SparcSignature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        e = Location::regOf(8);
    }

    Signature::addReturn(type, e);
}


void SparcSignature::addParameter(const QString& name, const SharedExp& e,
                                  SharedType type, const QString& boundMax)
{
    Signature::addParameter(name, e ? e : getArgumentExp(m_params.size()), type, boundMax);
}


SharedExp SparcSignature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    SharedExp e;

    if (n >= 6) {
        // SPARCs pass the seventh and subsequent parameters at m[%sp+92],
        // m[%esp+96], etc.
        e = Location::memOf(Binary::get(opPlus,
                                        Location::regOf(14), // %o6 == %sp
                                        Const::get(92 + (n - 6) * 4)));
    }
    else {
        e = Location::regOf(8 + n);
    }

    return e;
}


std::shared_ptr<Signature> SparcSignature::promote(UserProc * /*p*/)
{
    // no promotions from here up, obvious example would be name mangling
    return shared_from_this();
}


SharedExp SparcSignature::getProven(SharedExp left) const
{
    if (left->isRegOfK()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r)
        {
        // These registers are preserved in Sparc: i0-i7 (24-31), sp (14)
        case 14: // sp
        case 24:
        case 25:
        case 26:
        case 27: // i0-i3
        case 28:
        case 29:
        case 30:
        case 31: // i4-i7
            // NOTE: Registers %g2 to %g4 are NOT preserved in ordinary application (non library) code
            return left;
        }
    }

    return nullptr;
}


bool SparcSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfK()) {
        int r = e->access<Const, 1>()->getInt();

        switch (r)
        {
        // These registers are preserved in Sparc: i0-i7 (24-31), sp (14)
        case 14: // sp
        case 24:
        case 25:
        case 26:
        case 27: // i0-i3
        case 28:
        case 29:
        case 30:
        case 31: // i4-i7
            // NOTE: Registers %g2 to %g4 are NOT preserved in ordinary application (non library) code
            return true;

        default:
            return false;
        }
    }

    return false;
}


void SparcSignature::getLibraryDefines(StatementList& defs)
{
    if (defs.size() > 0) {
        return; // Do only once
    }

    for (int r = 8; r <= 15; ++r) {
        defs.append(new ImplicitAssign(Location::regOf(r))); // o0-o7 (r8-r15) modified
    }
}


SharedExp SparcLibSignature::getProven(SharedExp left) const
{
    if (left->isRegOfK()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r)
        {
        // These registers are preserved in Sparc: i0-i7 (24-31), sp (14)
        case 14:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        // Also the "application global registers" g2-g4 (2-4) (preserved
        // by library functions, but apparently don't have to be preserved
        // by application code)
        case 2:
        case 3:
        case 4: // g2-g4
            // The system global registers (g5-g7) are also preserved, but
            // should never be changed in an application anyway
            return left;
        }
    }

    return nullptr;
}

bool SparcSignature::qualified(UserProc *p, Signature& /*candidate*/)
{
    LOG_VERBOSE2("Consider promotion to stdc sparc signature for %1", p->getName());

    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::SPARC) {
        return false;
    }

    LOG_VERBOSE2("Promoted to StdC::SparcSignature");

    return true;
}



bool SparcSignature::isAddrOfStackLocal(int spIndex, const SharedConstExp& e) const
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

static Unary spPlus64(opMemOf, Binary::get(opPlus, Location::regOf(14), Const::get(64)));

bool SparcSignature::returnCompare(const Assignment& a, const Assignment& b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();

    // %o0 (r8) is the preferred return location
    if (la->isRegN(8)) {
        return true; // r24 is less than anything
    }

    if (lb->isRegN(8)) {
        return false; // Nothing is less than r24
    }

    // Next best is %f0 (r32)
    if (la->isRegN(32)) {
        return true; // r32 is less than anything that's left
    }

    if (lb->isRegN(32)) {
        return false; // Nothing left is less than r32
    }

    // Next best is %f0-1 (r64)
    if (la->isRegN(64)) {
        return true; // r64 is less than anything that's left
    }

    if (lb->isRegN(64)) {
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


bool SparcSignature::argumentCompare(const Assignment& a, const Assignment& b) const
{
    SharedConstExp la = a.getLeft();
    SharedConstExp lb = b.getLeft();
    // %o0-$o5 (r8-r13) are the preferred argument locations
    int ra = 0, rb = 0;

    if (la->isRegOf()) {
        int r = la->access<Const, 1>()->getInt();

        if ((r >= 8) && (r <= 13)) {
            ra = r;
        }
    }

    if (lb->isRegOf()) {
        int r = lb->access<Const, 1>()->getInt();

        if ((r >= 8) && (r <= 13)) {
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

    const int ma = Util::getStackOffset(la, 30);
    const int mb = Util::getStackOffset(lb, 30);

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
}

