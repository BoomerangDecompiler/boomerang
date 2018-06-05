#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Win32Signature.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/Prog.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/db/statements/ImplicitAssign.h"


namespace CallingConvention
{
Win32Signature::Win32Signature(const QString& name)
    : Signature(name)
{
    Signature::addReturn(Location::regOf(PENT_REG_ESP));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                Location::regOf(PENT_REG_ESP), nullptr);
}


Win32Signature::Win32Signature(Signature& old)
    : Signature(old)
{
}


Win32TcSignature::Win32TcSignature(const QString& name)
    : Win32Signature(name)
{
    Signature::addReturn(Location::regOf(PENT_REG_ESP));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                Location::regOf(PENT_REG_ESP), nullptr);
}


Win32TcSignature::Win32TcSignature(Signature& old)
    : Win32Signature(old)
{
}


std::shared_ptr<Signature> Win32Signature::clone() const
{
    Win32Signature *n = new Win32Signature(m_name);

    Util::clone(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    Util::clone(m_returns, n->m_returns);

    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;

    return std::shared_ptr<Signature>(n);
}


std::shared_ptr<Signature> Win32TcSignature::clone() const
{
    Win32TcSignature *n = new Win32TcSignature(m_name);

    Util::clone(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    Util::clone(m_returns, n->m_returns);

    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;

    return std::shared_ptr<Signature>(n);
}


bool Win32Signature::operator==(const Signature& other) const
{
    return Signature::operator==(other);
}


static SharedExp savedReturnLocation = Location::memOf(Location::regOf(PENT_REG_ESP));
static SharedExp stackPlusFour       = Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(4));

bool Win32Signature::qualified(UserProc *p, Signature& /*candidate*/)
{
    Platform plat = p->getProg()->getFrontEndId();

    if ((plat != Platform::PENTIUM) || !p->getProg()->isWin32()) {
        return false;
    }

    LOG_VERBOSE2("Consider promotion to stdc win32 signature for %1", p->getName());

    bool      gotcorrectret1, gotcorrectret2 = false;
    SharedExp provenPC = p->getProven(Terminal::get(opPC));
    gotcorrectret1 = provenPC && (*provenPC == *savedReturnLocation);

    if (gotcorrectret1) {
        LOG_VERBOSE2("got pc = m[r[28]]");

        SharedExp provenSP = p->getProven(Location::regOf(PENT_REG_ESP));
        gotcorrectret2 = provenSP && *provenSP == *stackPlusFour;

        if (gotcorrectret2) {
            LOG_VERBOSE2("Got r[28] = r[28] + 4");
        }
    }

    LOG_VERBOSE2("Qualified: %1", (gotcorrectret1 && gotcorrectret2));

    return gotcorrectret1 && gotcorrectret2;
}


void Win32Signature::addReturn(SharedType type, SharedExp e)
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


void Win32Signature::addParameter(const QString& name, const SharedExp& e,
                                  SharedType type, const QString& boundMax)
{
    Signature::addParameter(name, e ? e : getArgumentExp(m_params.size()), type, boundMax);
}


SharedExp Win32Signature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    SharedExp esp = Location::regOf(PENT_REG_ESP);

    if ((m_params.size() != 0) && (*m_params[0]->getExp() == *esp)) {
        n--;
    }

    SharedExp e = Location::memOf(Binary::get(opPlus, esp, Const::get((n + 1) * 4)));
    return e;
}


SharedExp Win32TcSignature::getArgumentExp(int n) const
{
    if (n < static_cast<int>(m_params.size())) {
        return Signature::getArgumentExp(n);
    }

    SharedExp esp = Location::regOf(PENT_REG_ESP);

    if (!m_params.empty() && (*m_params[0]->getExp() == *esp)) {
        n--;
    }

    if (n == 0) {
        // It's the first parameter, register ecx
        return Location::regOf(PENT_REG_ECX);
    }

    // Else, it is m[esp+4n)]
    SharedExp e = Location::memOf(Binary::get(opPlus, esp, Const::get(n * 4)));
    return e;
}


std::shared_ptr<Signature> Win32Signature::promote(UserProc * /*p*/)
{
    // no promotions from win32 signature up, yet.
    // a possible thing to investigate would be COM objects
    return shared_from_this();
}


SharedExp Win32Signature::getProven(SharedExp left) const
{
    size_t nparams = m_params.size();

    if ((nparams > 0) && (*m_params[0]->getExp() == *Location::regOf(PENT_REG_ESP))) {
        nparams--;
    }

    if (left->isRegOfConst()) {
        switch (left->access<Const, 1>()->getInt())
        {
            // Note: assumes callee pop... not true for cdecl functions!
        case PENT_REG_ESP: return Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(4 + nparams * 4));
        case PENT_REG_EBX: return Location::regOf(PENT_REG_EBX);
        case PENT_REG_EBP: return Location::regOf(PENT_REG_EBP);
        case PENT_REG_ESI: return Location::regOf(PENT_REG_ESI);
        case PENT_REG_EDI: return Location::regOf(PENT_REG_EDI);
            // there are other things that must be preserved here, look at calling convention
        }
    }

    return nullptr;
}


bool Win32Signature::isPreserved(SharedExp e) const
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
        case PENT_REG_BH:  // bh
            return true;
        default:
            return false;
        }
    }

    return false;
}


void Win32Signature::getLibraryDefines(StatementList& defs)
{
    if (defs.size()) {
        return;                           // Do only once
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


SharedExp Win32TcSignature::getProven(SharedExp left) const
{
    if (left->isRegOfConst()) {
        if (left->access<Const, 1>()->getInt() == PENT_REG_ESP) {
            int nparams = m_params.size();

            if ((nparams > 0) && (*m_params[0]->getExp() == *Location::regOf(PENT_REG_ESP))) {
                nparams--;
            }

            // r28 += 4 + nparams*4 - 4        (-4 because ecx is register param)
            return Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(4 + nparams * 4 - 4));
        }
    }

    // Else same as for standard Win32 signature
    return Win32Signature::getProven(left);
}

}
