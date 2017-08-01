/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       signature.cpp
 * \brief   Implementation of the classes that describe a procedure signature
 ******************************************************************************/

#include "Signature.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/core/BinaryFileFactory.h"

#include "boomerang/db/Signature.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/Proc.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/RefExp.h"

#include "boomerang/db/Managed.h"

#include "boomerang/type/Type.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"


#include <QtCore/QDebug>

#include <cassert>
#include <string>
#include <cstring>
#include <sstream>


Parameter::Parameter(SharedType type, const QString& name, SharedExp exp, const QString& boundMax)
    : m_type(type)
    , m_name(name)
    , m_exp(exp)
    , m_boundMax(boundMax)
{
}


QString Signature::getPlatformName(Platform plat)
{
    switch (plat)
    {
    case Platform::PENTIUM:
        return "pentium";

    case Platform::SPARC:
        return "sparc";

    case Platform::M68K:
        return "m68k";

    case Platform::PARISC:
        return "parisc";

    case Platform::PPC:
        return "ppc";

    case Platform::MIPS:
        return "mips";

    case Platform::ST20:
        return "st20";

    default:
        return "???";
    }
}


QString Signature::getConventionName(CallConv cc)
{
    switch (cc)
    {
    case CallConv::C:
        return "stdc";

    case CallConv::Pascal:
        return "pascal";

    case CallConv::ThisCall:
        return "thiscall";

    case CallConv::FastCall:
        return "fastcall";

    default:
        return "??";
    }
}


namespace CallingConvention
{
/// Win32Signature is for non-thiscall signatures: all parameters pushed
class Win32Signature : public Signature
{
public:
    Win32Signature(const QString& nam);
    Win32Signature(Signature& old);
    virtual ~Win32Signature() {}

    virtual std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;

    static bool qualified(UserProc *p, Signature& candidate);

    void addReturn(SharedType type, SharedExp e = nullptr) override;
    void addParameter(SharedType type, const QString& nam = QString::null, const SharedExp& e = nullptr,
                      const QString& boundMax = "") override;
    virtual SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;
    virtual SharedExp getStackWildcard() const override;

    virtual int getStackRegister() const noexcept(false) override { return 28; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList *defs) override; // Set list of locations def'd by library calls

    virtual bool isPromoted()        const override { return true; }
    virtual Platform getPlatform()   const override { return Platform::PENTIUM; }
    virtual CallConv getConvention() const override { return CallConv::Pascal; }
};


/// Win32TcSignature is for "thiscall" signatures, i.e. those that have register ecx as the first parameter
/// Only needs to override a few member functions; the rest can inherit from Win32Signature
class Win32TcSignature : public Win32Signature
{
public:
    Win32TcSignature(const QString& nam);
    Win32TcSignature(Signature& old);

    virtual SharedExp getArgumentExp(int n) const override;
    virtual SharedExp getProven(SharedExp left) const override;

    virtual std::shared_ptr<Signature> clone() const override;

    virtual Platform getPlatform() const override { return Platform::PENTIUM; }
    virtual CallConv getConvention() const override { return CallConv::ThisCall; }
};

namespace StdC
{
class PentiumSignature : public Signature
{
public:
    PentiumSignature(const QString& nam);
    PentiumSignature(Signature& old);
    virtual ~PentiumSignature() {}
    virtual std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;

    /// FIXME: This needs changing. Would like to check that pc=pc and sp=sp
    /// (or maybe sp=sp+4) for qualifying procs. Need work to get there
    static bool qualified(UserProc *p, Signature&);

    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    virtual void addParameter(SharedType type, const QString& nam = QString::null, const SharedExp& e = nullptr,
                              const QString& boundMax = "") override;
    virtual SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;
    virtual SharedExp getStackWildcard() const override;

    virtual int getStackRegister() const noexcept(false) override { return 28; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc

    /// Return a list of locations defined by library calls
    virtual void setLibraryDefines(StatementList *defs) override;

    virtual bool isPromoted() const override { return true; }
    virtual Platform getPlatform() const override { return Platform::PENTIUM; }
    virtual CallConv getConvention() const override { return CallConv::C; }
    virtual bool returnCompare(Assignment& a, Assignment& b) const override;
    virtual bool argumentCompare(Assignment& a, Assignment& b) const override;
};


class SparcSignature : public Signature
{
public:
    SparcSignature(const QString& nam);
    SparcSignature(Signature& old);
    virtual ~SparcSignature() {}

    virtual std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;
    static bool qualified(UserProc *p, Signature&);

    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    virtual void addParameter(SharedType type, const QString& nam = QString::null, const SharedExp& e = nullptr,
                              const QString& boundMax = "") override;
    virtual SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;
    virtual SharedExp getStackWildcard() const override;

    virtual int getStackRegister() const noexcept(false) override { return 14; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc

    /// Return a list of locations defined by library calls
    virtual void setLibraryDefines(StatementList *defs) override;

    /// Stack offsets can be negative (inherited) or positive:
    virtual bool isLocalOffsetPositive() const override { return true; }

    /// An override for testing locals
    /// An override for the SPARC: [sp+0] .. [sp+88] are local variables (effectively), but [sp + >=92] are memory parameters
    virtual bool isAddrOfStackLocal(Prog *prog, const SharedExp& e) const override;

    virtual bool isPromoted() const override { return true; }
    virtual Platform getPlatform() const override { return Platform::SPARC; }
    virtual CallConv getConvention() const override { return CallConv::C; }
    virtual bool returnCompare(Assignment& a, Assignment& b) const override;
    virtual bool argumentCompare(Assignment& a, Assignment& b) const override;
};


class SparcLibSignature : public SparcSignature
{
public:
    SparcLibSignature(const QString& nam)
        : SparcSignature(nam) {}
    SparcLibSignature(Signature& old);

    virtual std::shared_ptr<Signature> clone() const override;
    virtual SharedExp getProven(SharedExp left) const override;
};

class PPCSignature : public Signature
{
public:
    PPCSignature(const QString& name);
    PPCSignature(Signature& old);
    virtual ~PPCSignature() {}

    virtual std::shared_ptr<Signature> clone() const override;

    static bool qualified(UserProc *p, Signature&);
    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    virtual SharedExp getArgumentExp(int n) const override;
    virtual void addParameter(SharedType type, const QString& nam = QString::null, const SharedExp& e = nullptr,
                              const QString& boundMax = "") override;
    virtual SharedExp getStackWildcard() const override;

    virtual int getStackRegister() const noexcept(false) override { return 1; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList *defs) override; // Set list of locations def'd by library calls

    virtual bool isLocalOffsetPositive() const override { return true; }
    virtual bool isPromoted() const override { return true; }
    virtual Platform getPlatform() const override { return Platform::PPC; }
    virtual CallConv getConvention() const override { return CallConv::C; }

    std::shared_ptr<Signature> promote(UserProc * /*p*/) override
    {
        // No promotions from here up, obvious idea would be c++ name mangling
        return shared_from_this();
    }
};

class MIPSSignature : public Signature
{
public:
    MIPSSignature(const QString& name);
    MIPSSignature(Signature& old);
    virtual ~MIPSSignature() {}

    virtual std::shared_ptr<Signature> clone() const override;

    static bool qualified(UserProc *p, Signature&);
    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    virtual SharedExp getArgumentExp(int n) const override;
    virtual void addParameter(SharedType type, const QString& nam = QString::null, const SharedExp& e = nullptr,
                              const QString& boundMax = "") override;
    virtual SharedExp getStackWildcard() const override;

    virtual int getStackRegister() const noexcept(false) override { return 29; }
    virtual SharedExp getProven(SharedExp left) const override;

    // Return whether e is preserved by this proc
    virtual bool isPreserved(SharedExp e) const override;

    /// Return a list of locations defined by library calls
    virtual void setLibraryDefines(StatementList *defs) override;

    virtual bool isLocalOffsetPositive() const override { return true; }
    virtual bool isPromoted() const override { return true; }
    virtual Platform getPlatform() const override { return Platform::MIPS; }
    virtual CallConv getConvention() const override { return CallConv::C; }
};


class ST20Signature : public Signature
{
public:
    ST20Signature(const QString& name);
    ST20Signature(Signature& old);
    virtual ~ST20Signature() {}
    std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;
    static bool qualified(UserProc *p, Signature&);

    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    void addParameter(SharedType type, const QString& nam = QString::null, const SharedExp& e = nullptr,
                      const QString& boundMax = "") override;
    SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;
    virtual SharedExp getStackWildcard() const override;

    virtual int getStackRegister() const noexcept(false) override { return 3; }
    virtual SharedExp getProven(SharedExp left) const override;

    virtual bool isPromoted() const override { return true; }

    // virtual bool isLocalOffsetPositive() {return true;}

    virtual Platform getPlatform() const override { return Platform::ST20; }
    virtual CallConv getConvention() const override { return CallConv::C; }
};
} // namespace StdC
} // namespace CallingConvention


CallingConvention::Win32Signature::Win32Signature(const QString& nam)
    : Signature(nam)
{
    Signature::addReturn(Location::regOf(28));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                Location::regOf(28), nullptr);
}


CallingConvention::Win32Signature::Win32Signature(Signature& old)
    : Signature(old)
{
}


CallingConvention::Win32TcSignature::Win32TcSignature(const QString& nam)
    : Win32Signature(nam)
{
    Signature::addReturn(Location::regOf(28));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                Location::regOf(28), nullptr);
}


CallingConvention::Win32TcSignature::Win32TcSignature(Signature& old)
    : Win32Signature(old)
{
}


template<class Cloneable>
static void cloneVec(const std::vector<std::shared_ptr<Cloneable> >& from, std::vector<std::shared_ptr<Cloneable> >& to)
{
    unsigned n = from.size();

    to.resize(n);

    for (unsigned i = 0; i < n; i++) {
        to[i] = from[i]->clone();
    }
}


std::shared_ptr<Parameter> Parameter::clone() const
{
    return std::make_shared<Parameter>(m_type->clone(), m_name, m_exp->clone(), m_boundMax);
}


void Parameter::setBoundMax(const QString& nam)
{
    m_boundMax = nam;
}


std::shared_ptr<Signature> CallingConvention::Win32Signature::clone() const
{
    Win32Signature *n = new Win32Signature(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_rettype       = m_rettype->clone();
    n->m_preferredName = m_preferredName;

    if (m_preferredReturn) {
        n->m_preferredReturn = m_preferredReturn->clone();
    }
    else {
        n->m_preferredReturn = nullptr;
    }

    n->m_preferredParams = m_preferredParams;
    return std::shared_ptr<Signature>(n);
}


std::shared_ptr<Signature> CallingConvention::Win32TcSignature::clone() const
{
    Win32TcSignature *n = new Win32TcSignature(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_rettype       = m_rettype->clone();
    n->m_preferredName = m_preferredName;

    if (m_preferredReturn) {
        n->m_preferredReturn = m_preferredReturn->clone();
    }
    else {
        n->m_preferredReturn = nullptr;
    }

    n->m_preferredParams = m_preferredParams;
    return std::shared_ptr<Signature>(n);
}


bool CallingConvention::Win32Signature::operator==(const Signature& other) const
{
    return Signature::operator==(other);
}


static SharedExp savedReturnLocation = Location::memOf(Location::regOf(28));
static SharedExp stackPlusFour       = Binary::get(opPlus, Location::regOf(28), Const::get(4));

bool CallingConvention::Win32Signature::qualified(UserProc *p, Signature& /*candidate*/)
{
    Platform plat = p->getProg()->getFrontEndId();

    if ((plat != Platform::PENTIUM) || !p->getProg()->isWin32()) {
        return false;
    }

    LOG_VERBOSE("Consider promotion to stdc win32 signature for %1", p->getName());

    bool      gotcorrectret1, gotcorrectret2 = false;
    SharedExp provenPC = p->getProven(Terminal::get(opPC));
    gotcorrectret1 = provenPC && (*provenPC == *savedReturnLocation);

    if (gotcorrectret1) {
        LOG_VERBOSE("got pc = m[r[28]]");

        SharedExp provenSP = p->getProven(Location::regOf(28));
        gotcorrectret2 = provenSP && *provenSP == *stackPlusFour;

        if (gotcorrectret2) {
            LOG_VERBOSE("Got r[28] = r[28] + 4");
        }
    }

    LOG_VERBOSE("Qualified: %1", (gotcorrectret1 && gotcorrectret2));

    return gotcorrectret1 && gotcorrectret2;
}


void CallingConvention::Win32Signature::addReturn(SharedType type, SharedExp e)
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


void CallingConvention::Win32Signature::addParameter(SharedType type, const QString& nam, const SharedExp& e,
                                                     const QString& boundMax)
{
    Signature::addParameter(type, nam, e ? e : getArgumentExp(m_params.size()), boundMax);
}


SharedExp CallingConvention::Win32Signature::getArgumentExp(int n) const
{
    if (n < (int)m_params.size()) {
        return Signature::getArgumentExp(n);
    }

    SharedExp esp = Location::regOf(28);

    if ((m_params.size() != 0) && (*m_params[0]->getExp() == *esp)) {
        n--;
    }

    SharedExp e = Location::memOf(Binary::get(opPlus, esp, Const::get((n + 1) * 4)));
    return e;
}


SharedExp CallingConvention::Win32TcSignature::getArgumentExp(int n) const
{
    if (n < (int)m_params.size()) {
        return Signature::getArgumentExp(n);
    }

    SharedExp esp = Location::regOf(28);

    if (!m_params.empty() && (*m_params[0]->getExp() == *esp)) {
        n--;
    }

    if (n == 0) {
        // It's the first parameter, register ecx
        return Location::regOf(25);
    }

    // Else, it is m[esp+4n)]
    SharedExp e = Location::memOf(Binary::get(opPlus, esp, Const::get(n * 4)));
    return e;
}


std::shared_ptr<Signature> CallingConvention::Win32Signature::promote(UserProc * /*p*/)
{
    // no promotions from win32 signature up, yet.
    // a possible thing to investigate would be COM objects
    return shared_from_this();
}


SharedExp CallingConvention::Win32Signature::getStackWildcard() const
{
    // Note: m[esp + -8] is simplified to m[esp - 8] now
    return Location::memOf(Binary::get(opMinus, Location::regOf(28), Terminal::get(opWild)));
}


SharedExp CallingConvention::Win32Signature::getProven(SharedExp left) const
{
    size_t nparams = m_params.size();

    if ((nparams > 0) && (*m_params[0]->getExp() == *Location::regOf(28))) {
        nparams--;
    }

    if (left->isRegOfK()) {
        switch (left->access<Const, 1>()->getInt())
        {
        case 28: // esp
            // Note: assumes callee pop... not true for cdecl functions!
            return Binary::get(opPlus, Location::regOf(28), Const::get(4 + nparams * 4));

        case 27: // ebx
            return Location::regOf(27);

        case 29: // ebp
            return Location::regOf(29);

        case 30: // esi
            return Location::regOf(30);

        case 31: // edi
            return Location::regOf(31);
            // there are other things that must be preserved here, look at calling convention
        }
    }

    return nullptr;
}


bool CallingConvention::Win32Signature::isPreserved(SharedExp e) const
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


void CallingConvention::Win32Signature::setLibraryDefines(StatementList *defs)
{
    if (defs->size()) {
        return;                           // Do only once
    }

    auto       r24 = Location::regOf(24); // eax
    SharedType ty  = SizeType::get(32);

    if (m_returns.size() > 1) {                  // Ugh - note the stack pointer is the first return still
        ty = m_returns[1]->m_type;
    }

    defs->append(new ImplicitAssign(ty, r24));             // eax
    defs->append(new ImplicitAssign(Location::regOf(25))); // ecx
    defs->append(new ImplicitAssign(Location::regOf(26))); // edx
    defs->append(new ImplicitAssign(Location::regOf(28))); // esp
}


SharedExp CallingConvention::Win32TcSignature::getProven(SharedExp left) const
{
    if (left->isRegOfK()) {
        if (left->access<Const, 1>()->getInt() == 28) {
            int nparams = m_params.size();

            if ((nparams > 0) && (*m_params[0]->getExp() == *Location::regOf(28))) {
                nparams--;
            }

            // r28 += 4 + nparams*4 - 4        (-4 because ecx is register param)
            return Binary::get(opPlus, Location::regOf(28), Const::get(4 + nparams * 4 - 4));
        }
    }

    // Else same as for standard Win32 signature
    return Win32Signature::getProven(left);
}


CallingConvention::StdC::PentiumSignature::PentiumSignature(const QString& nam)
    : Signature(nam)
{
    Signature::addReturn(Location::regOf(28));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                 Location::regOf(28), nullptr);
}


CallingConvention::StdC::PentiumSignature::PentiumSignature(Signature& old)
    : Signature(old)
{
}


std::shared_ptr<Signature> CallingConvention::StdC::PentiumSignature::clone() const
{
    PentiumSignature *n = new PentiumSignature(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_rettype       = m_rettype->clone();
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


bool CallingConvention::StdC::PentiumSignature::operator==(const Signature& other) const
{
    return Signature::operator==(other);
}


bool CallingConvention::StdC::PentiumSignature::qualified(UserProc *p, Signature& /*candidate*/)
{
    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::PENTIUM) {
        return false;
    }

    LOG_VERBOSE("Consider promotion to stdc pentium signature for %1", p->getName());

#if 1
    LOG_VERBOSE("Promotion qualified: always true");
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


void CallingConvention::StdC::PentiumSignature::addReturn(SharedType type, SharedExp e)
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


void CallingConvention::StdC::PentiumSignature::addParameter(SharedType type, const QString& nam, const SharedExp& e,
                                                             const QString& boundMax)
{
    Signature::addParameter(type, nam, e ? e : getArgumentExp(m_params.size()), boundMax);
}


SharedExp CallingConvention::StdC::PentiumSignature::getArgumentExp(int n) const
{
    if (n < (int)m_params.size()) {
        return Signature::getArgumentExp(n);
    }

    SharedExp esp = Location::regOf(28);

    if ((m_params.size() != 0) && (*m_params[0]->getExp() == *esp)) {
        n--;
    }

    SharedExp e = Location::memOf(Binary::get(opPlus, esp, Const::get((n + 1) * 4)));
    return e;
}


std::shared_ptr<Signature> CallingConvention::StdC::PentiumSignature::promote(UserProc * /*p*/)
{
    // No promotions from here up, obvious idea would be c++ name mangling
    return shared_from_this();
}


SharedExp CallingConvention::StdC::PentiumSignature::getStackWildcard() const
{
    // Note: m[esp + -8] is simplified to m[esp - 8] now
    return Location::memOf(Binary::get(opMinus, Location::regOf(28), Terminal::get(opWild)));
}


SharedExp CallingConvention::StdC::PentiumSignature::getProven(SharedExp left) const
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


bool CallingConvention::StdC::PentiumSignature::isPreserved(SharedExp e) const
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


void CallingConvention::StdC::PentiumSignature::setLibraryDefines(StatementList *defs)
{
    if (defs->size()) {
        return;                           // Do only once
    }

    auto       r24 = Location::regOf(24); // eax
    SharedType ty  = SizeType::get(32);

    if (m_returns.size() > 1) {                  // Ugh - note the stack pointer is the first return still
        ty = m_returns[1]->m_type;
    }

    defs->append(new ImplicitAssign(ty, r24));             // eax
    defs->append(new ImplicitAssign(Location::regOf(25))); // ecx
    defs->append(new ImplicitAssign(Location::regOf(26))); // edx
    defs->append(new ImplicitAssign(Location::regOf(28))); // esp
}


CallingConvention::StdC::PPCSignature::PPCSignature(const QString& nam)
    : Signature(nam)
{
    Signature::addReturn(Location::regOf(1));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "r1",
    //                                 Location::regOf(1), nullptr);
    // FIXME: Should also add m[r1+4] as an implicit parameter? Holds return address
}


CallingConvention::StdC::PPCSignature::PPCSignature(Signature& old)
    : Signature(old)
{
}


std::shared_ptr<Signature> CallingConvention::StdC::PPCSignature::clone() const
{
    PPCSignature *n = new PPCSignature(m_name);

    cloneVec(m_params, n->m_params);
    // n->implicitParams = implicitParams;
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_rettype       = m_rettype->clone();
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


SharedExp CallingConvention::StdC::PPCSignature::getArgumentExp(int n) const
{
    if (n < (int)m_params.size()) {
        return Signature::getArgumentExp(n);
    }

    SharedExp e;

    if (n >= 8) {
        // PPCs pass the ninth and subsequent parameters at m[%r1+8],
        // m[%r1+12], etc.
        e = Location::memOf(Binary::get(opPlus, Location::regOf(1), Const::get(8 + (n - 8) * 4)));
    }
    else {
        e = Location::regOf(3 + n);
    }

    return e;
}


void CallingConvention::StdC::PPCSignature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        e = Location::regOf(3);
    }

    Signature::addReturn(type, e);
}


void CallingConvention::StdC::PPCSignature::addParameter(SharedType type, const QString& nam,
                                                         const SharedExp& e, const QString& boundMax)
{
    Signature::addParameter(type, nam, e ? e : getArgumentExp(m_params.size()), boundMax);
}


SharedExp CallingConvention::StdC::PPCSignature::getStackWildcard() const
{
    // m[r1 - WILD]
    return Location::memOf(Binary::get(opMinus, Location::regOf(1), Terminal::get(opWild)));
}


SharedExp CallingConvention::StdC::PPCSignature::getProven(SharedExp left) const
{
    if (left->isRegOfK()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r)
        {
        case 1: // stack
            return left;
        }
    }

    return nullptr;
}


bool CallingConvention::StdC::PPCSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfK()) {
        int r = e->access<Const, 1>()->getInt();
        return r == 1;
    }

    return false;
}


// Return a list of locations defined by library calls
void CallingConvention::StdC::PPCSignature::setLibraryDefines(StatementList *defs)
{
    if (defs->size()) {
        return; // Do only once
    }

    for (int r = 3; r <= 12; ++r) {
        defs->append(new ImplicitAssign(Location::regOf(r))); // Registers 3-12 are volatile (caller save)
    }
}


/// ST20 signatures

CallingConvention::StdC::ST20Signature::ST20Signature(const QString& nam)
    : Signature(nam)
{
    Signature::addReturn(Location::regOf(3));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "sp", Location::regOf(3), nullptr);
    // FIXME: Should also add m[sp+0] as an implicit parameter? Holds return address
}


CallingConvention::StdC::ST20Signature::ST20Signature(Signature& old)
    : Signature(old)
{
}


std::shared_ptr<Signature> CallingConvention::StdC::ST20Signature::clone() const
{
    ST20Signature *n = new ST20Signature(m_name);

    n->m_params          = m_params;
    n->m_returns         = m_returns;
    n->m_ellipsis        = m_ellipsis;
    n->m_rettype         = m_rettype;
    n->m_preferredName   = m_preferredName;
    n->m_preferredReturn = m_preferredReturn;
    n->m_preferredParams = m_preferredParams;
    n->m_unknown         = m_unknown;

    return std::shared_ptr<Signature>(n);
}


bool CallingConvention::StdC::ST20Signature::operator==(const Signature& other) const
{
    return Signature::operator==(other);
}


SharedExp CallingConvention::StdC::ST20Signature::getArgumentExp(int n) const
{
    if (n < (int)m_params.size()) {
        return Signature::getArgumentExp(n);
    }

    // m[%sp+4], etc.
    SharedExp sp = Location::regOf(3);

    if ((m_params.size() != 0) && (*m_params[0]->getExp() == *sp)) {
        n--;
    }

    SharedExp e = Location::memOf(Binary::get(opPlus, sp, Const::get((n + 1) * 4)));
    return e;
}


void CallingConvention::StdC::ST20Signature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        e = Location::regOf(0);
    }

    Signature::addReturn(type, e);
}


std::shared_ptr<Signature> CallingConvention::StdC::ST20Signature::promote(UserProc * /*p*/)
{
    // No promotions from here up, obvious idea would be c++ name mangling
    return shared_from_this();
}


void CallingConvention::StdC::ST20Signature::addParameter(SharedType type, const QString& nam,
                                                          const SharedExp& e, const QString& boundMax)
{
    Signature::addParameter(type, nam, e ? e : getArgumentExp(m_params.size()), boundMax);
}


SharedExp CallingConvention::StdC::ST20Signature::getStackWildcard() const
{
    // m[r1 - WILD]
    return Location::memOf(Binary::get(opMinus, Location::regOf(3), Terminal::get(opWild)));
}


SharedExp CallingConvention::StdC::ST20Signature::getProven(SharedExp left) const
{
    if (left->isRegOfK()) {
        int r = left->access<Const, 1>()->getInt();

        switch (r)
        {
        case 3:
            // return Binary::get(opPlus, Location::regOf(3), Const::get(4));
            return left;

        case 0:
        case 1:
        case 2:
            // Registers A, B, and C are callee save
            return Location::regOf(r);
        }
    }

    return nullptr;
}


bool CallingConvention::StdC::ST20Signature::qualified(UserProc *p, Signature& /*candidate*/)
{
    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::ST20) {
        return false;
    }

    LOG_VERBOSE("Consider promotion to stdc st20 signature for %1", p->getName());

    return true;
}


CallingConvention::StdC::SparcSignature::SparcSignature(const QString& nam)
    : Signature(nam)
{
    Signature::addReturn(Location::regOf(14));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "sp",
    //                                Location::regOf(14), nullptr);
}


CallingConvention::StdC::SparcSignature::SparcSignature(Signature& old)
    : Signature(old)
{
}


std::shared_ptr<Signature> CallingConvention::StdC::SparcSignature::clone() const
{
    SparcSignature *n = new SparcSignature(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_rettype       = m_rettype->clone();
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


std::shared_ptr<Signature> CallingConvention::StdC::SparcLibSignature::clone() const
{
    SparcLibSignature *n = new SparcLibSignature(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_rettype       = m_rettype->clone();
    n->m_preferredName = m_preferredName;

    if (m_preferredReturn) {
        n->m_preferredReturn = m_preferredReturn->clone();
    }
    else {
        n->m_preferredReturn = nullptr;
    }

    n->m_preferredParams = m_preferredParams;
    return std::shared_ptr<Signature>(n);
}


bool CallingConvention::StdC::SparcSignature::operator==(const Signature& other) const
{
    return Signature::operator==(other);
}


bool CallingConvention::StdC::SparcSignature::qualified(UserProc *p, Signature& /*candidate*/)
{
    LOG_VERBOSE("Consider promotion to stdc sparc signature for %1", p->getName());

    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::SPARC) {
        return false;
    }

    LOG_VERBOSE("Promoted to StdC::SparcSignature");

    return true;
}


bool CallingConvention::StdC::PPCSignature::qualified(UserProc *p, Signature& /*candidate*/)
{
    LOG_VERBOSE("Consider promotion to stdc PPC signature for %1", p->getName());

    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::PPC) {
        return false;
    }

    LOG_VERBOSE("Promoted to StdC::PPCSignature (always qualifies)");

    return true;
}


CallingConvention::StdC::MIPSSignature::MIPSSignature(const QString& _name)
    : Signature(_name)
{
    Signature::addReturn(Location::regOf(2));
}


std::shared_ptr<Signature> CallingConvention::StdC::MIPSSignature::clone() const
{
    MIPSSignature *n = new MIPSSignature(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis      = m_ellipsis;
    n->m_rettype       = m_rettype->clone();
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


bool CallingConvention::StdC::MIPSSignature::qualified(UserProc *p, Signature& /*candidate*/)
{
    LOG_VERBOSE("Consider promotion to stdc MIPS signature for %1", p->getName());

    Platform plat = p->getProg()->getFrontEndId();

    if (plat != Platform::MIPS) {
        return false;
    }

    LOG_VERBOSE("Promoted to StdC::MIPSSignature (always qualifies)");

    return true;
}


void CallingConvention::StdC::MIPSSignature::addReturn(SharedType type, SharedExp e)
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


SharedExp CallingConvention::StdC::MIPSSignature::getArgumentExp(int n) const
{
    if (n < (int)m_params.size()) {
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
        e = Location::regOf((int)(8 + n));
    }

    return e;
}


void CallingConvention::StdC::MIPSSignature::addParameter(SharedType type, const QString& nam, const SharedExp& e, const QString& boundMax)
{
    Signature::addParameter(type, nam, e ? e : getArgumentExp(m_params.size()), boundMax);
}


SharedExp CallingConvention::StdC::MIPSSignature::getStackWildcard() const
{
    // m[%sp - WILD]
    return Location::memOf(Binary::get(opMinus, Location::regOf(getStackRegister()), Terminal::get(opWild)));
}


SharedExp CallingConvention::StdC::MIPSSignature::getProven(SharedExp left) const
{
    if (left->isRegOfK()) {
        int r = left->access<Const, 1>()->getInt();

        if (r == getStackRegister()) {
            return left;
        }
    }

    return nullptr;
}


bool CallingConvention::StdC::MIPSSignature::isPreserved(SharedExp e) const
{
    if (e->isRegOfK()) {
        int r = e->access<Const, 1>()->getInt();
        return r == getStackRegister();
    }

    return false;
}


void CallingConvention::StdC::MIPSSignature::setLibraryDefines(StatementList *defs)
{
    if (defs->size()) {
        return; // Do only once
    }

    for (int r = 16; r <= 23; ++r) {
        defs->append(new ImplicitAssign(Location::regOf(r))); // Registers 16-23 are volatile (caller save)
    }

    defs->append(new ImplicitAssign(Location::regOf(30)));
}


void CallingConvention::StdC::SparcSignature::addReturn(SharedType type, SharedExp e)
{
    if (type->isVoid()) {
        return;
    }

    if (e == nullptr) {
        e = Location::regOf(8);
    }

    Signature::addReturn(type, e);
}


void CallingConvention::StdC::SparcSignature::addParameter(SharedType type, const QString& nam,
                                                           const SharedExp& e, const QString& boundMax)
{
    Signature::addParameter(type, nam, e ? e : getArgumentExp(m_params.size()), boundMax);
}


SharedExp CallingConvention::StdC::SparcSignature::getArgumentExp(int n) const
{
    if (n < (int)m_params.size()) {
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


std::shared_ptr<Signature> CallingConvention::StdC::SparcSignature::promote(UserProc * /*p*/)
{
    // no promotions from here up, obvious example would be name mangling
    return shared_from_this();
}


SharedExp CallingConvention::StdC::SparcSignature::getStackWildcard() const
{
    return Location::memOf(Binary::get(opPlus, Location::regOf(14), Terminal::get(opWild)));
}


SharedExp CallingConvention::StdC::SparcSignature::getProven(SharedExp left) const
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


bool CallingConvention::StdC::SparcSignature::isPreserved(SharedExp e) const
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


void CallingConvention::StdC::SparcSignature::setLibraryDefines(StatementList *defs)
{
    if (defs->size()) {
        return; // Do only once
    }

    for (int r = 8; r <= 15; ++r) {
        defs->append(new ImplicitAssign(Location::regOf(r))); // o0-o7 (r8-r15) modified
    }
}


SharedExp CallingConvention::StdC::SparcLibSignature::getProven(SharedExp left) const
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


Signature::Signature(const QString& nam)
    : m_rettype(VoidType::get())
    , m_ellipsis(false)
    , m_unknown(true)
    , m_forced(false)
    , m_preferredReturn(nullptr)
{
    if (nam == nullptr) {
        m_name = "<ANON>";
    }
    else {
        m_name = nam;

        if (m_name == "__glutWarning") {
            qDebug() << m_name;
        }
    }
}


CustomSignature::CustomSignature(const QString& nam)
    : Signature(nam)
    , sp(0)
{
}


void CustomSignature::setSP(int nsp)
{
    sp = nsp;

    if (sp) {
        addReturn(Location::regOf(sp));
        // addImplicitParameter(PointerType::get(new IntegerType()), "sp",
        //                            Location::regOf(sp), nullptr);
    }
}


std::shared_ptr<Signature> Signature::clone() const
{
    auto n = std::make_shared<Signature>(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis        = m_ellipsis;
    n->m_rettype         = m_rettype->clone();
    n->m_preferredName   = m_preferredName;
    n->m_preferredReturn = m_preferredReturn ? m_preferredReturn->clone() : nullptr;
    n->m_preferredParams = m_preferredParams;
    n->m_unknown         = m_unknown;
    n->m_sigFile         = m_sigFile;
    return n;
}


std::shared_ptr<Signature> CustomSignature::clone() const
{
    CustomSignature *n = new CustomSignature(m_name);

    cloneVec(m_params, n->m_params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(m_returns, n->m_returns);
    n->m_ellipsis        = m_ellipsis;
    n->m_rettype         = m_rettype->clone();
    n->sp                = sp;
    n->m_forced          = m_forced;
    n->m_preferredName   = m_preferredName;
    n->m_preferredReturn = m_preferredReturn ? m_preferredReturn->clone() : nullptr;
    n->m_preferredParams = m_preferredParams;
    n->m_unknown         = m_unknown;
    n->m_sigFile         = m_sigFile;
    return std::shared_ptr<Signature>(n);
}


bool Signature::operator==(const Signature& other) const
{
    // if (name != other.name) return false;        // MVE: should the name be significant? I'm thinking no
    if (m_params.size() != other.m_params.size()) {
        return false;
    }

    // Only care about the first return location (at present)
    for (auto it1 = m_params.begin(), it2 = other.m_params.begin(); it1 != m_params.end(); it1++, it2++) {
        if (!(**it1 == **it2)) {
            return false;
        }
    }

    if (m_returns.size() != other.m_returns.size()) {
        return false;
    }

    for (auto rr1 = m_returns.begin(), rr2 = other.m_returns.begin(); rr1 != m_returns.end(); ++rr1, ++rr2) {
        if (!(**rr1 == **rr2)) {
            return false;
        }
    }

    return true;
}


QString Signature::getName() const
{
    return m_name;
}


void Signature::setName(const QString& nam)
{
    m_name = nam;
}


void Signature::addParameter(const char *nam /*= nullptr*/)
{
    addParameter(VoidType::get(), nam);
}


void Signature::addParameter(const SharedExp& e, SharedType ty)
{
    addParameter(ty, nullptr, e);
}


void Signature::addParameter(SharedType type, const QString& nam /*= nullptr*/, const SharedExp& e /*= nullptr*/,
                             const QString& boundMax /*= ""*/)
{
    if (e == nullptr) {
        // Else get infinite mutual recursion with the below proc
        LOG_FATAL("No expression for parameter %1 %2",
                  type ? type->getCtype() : "<notype>",
                  !nam.isNull() ? qPrintable(nam) : "<noname>");
    }

    QString s;
    QString new_name = nam;

    if (nam.isNull()) {
        size_t n  = m_params.size() + 1;
        bool   ok = false;

        while (!ok) {
            s  = QString("param%1").arg(n);
            ok = true;

            for (auto& elem : m_params) {
                if (s == elem->getName()) {
                    ok = false;
                    break;
                }
            }

            n++;
        }

        new_name = s;
    }

    addParameter(std::make_shared<Parameter>(type, new_name, e, boundMax));
    // addImplicitParametersFor(p);
}


void Signature::addParameter(std::shared_ptr<Parameter> param)
{
    SharedType ty  = param->getType();
    QString    nam = param->getName();
    SharedExp  e   = param->getExp();

    if (nam.isEmpty()) {
        nam = QString::null;
    }

    if ((ty == nullptr) || (e == nullptr) || nam.isNull()) {
        addParameter(ty, nam, e, param->getBoundMax());
    }
    else {
        m_params.push_back(param);
    }
}


void Signature::removeParameter(const SharedExp& e)
{
    int i = findParam(e);

    if (i != -1) {
        removeParameter(i);
    }
}


void Signature::removeParameter(size_t i)
{
    for (size_t j = i + 1; j < m_params.size(); j++) {
        m_params[j - 1] = m_params[j];
    }

    m_params.resize(m_params.size() - 1);
}


void Signature::setNumParams(size_t n)
{
    if (n < m_params.size()) {
        // truncate
        m_params.erase(m_params.begin() + n, m_params.end());
    }
    else {
        for (size_t i = m_params.size(); i < n; i++) {
            addParameter();
        }
    }
}


const QString& Signature::getParamName(size_t n) const
{
    assert(n < m_params.size());
    return m_params[n]->getName();
}


SharedExp Signature::getParamExp(int n) const
{
    assert(n < (int)m_params.size());
    return m_params[n]->getExp();
}


SharedType Signature::getParamType(int n) const
{
    // assert(n < (int)params.size() || ellipsis);
    // With recursion, parameters not set yet. Hack for now:
    if (n >= (int)m_params.size()) {
        return nullptr;
    }

    return m_params[n]->getType();
}


QString Signature::getParamBoundMax(int n) const
{
    if (n >= (int)m_params.size()) {
        return QString::null;
    }

    QString s = m_params[n]->getBoundMax();

    if (s.isEmpty()) {
        return QString::null;
    }

    return s;
}


void Signature::setParamType(int n, SharedType ty)
{
    m_params[n]->setType(ty);
}


void Signature::setParamType(const char *nam, SharedType ty)
{
    int idx = findParam(nam);

    if (idx == -1) {
        LOG_WARN("Could not set type for unknown parameter %1", nam);
        return;
    }

    m_params[idx]->setType(ty);
}


void Signature::setParamType(const SharedExp& e, SharedType ty)
{
    int idx = findParam(e);

    if (idx == -1) {
        LOG_WARN("Could not set type for unknown parameter expression %1", e);
        return;
    }

    m_params[idx]->setType(ty);
}


void Signature::setParamName(int n, const char *_name)
{
    m_params[n]->setName(_name);
}


void Signature::setParamExp(int n, SharedExp e)
{
    m_params[n]->setExp(e);
}


int Signature::findParam(const SharedExp& e) const
{
    for (unsigned i = 0; i < getNumParams(); i++) {
        if (*getParamExp(i) == *e) {
            return i;
        }
    }

    return -1;
}


void Signature::renameParam(const QString& oldName, const char *newName)
{
    for (unsigned i = 0; i < getNumParams(); i++) {
        if (m_params[i]->getName() == oldName) {
            m_params[i]->setName(newName);
            break;
        }
    }
}


int Signature::findParam(const QString& nam) const
{
    for (unsigned i = 0; i < getNumParams(); i++) {
        if (getParamName(i) == nam) {
            return i;
        }
    }

    return -1;
}


int Signature::findReturn(SharedExp e) const
{
    for (unsigned i = 0; i < getNumReturns(); i++) {
        if (*m_returns[i]->m_exp == *e) {
            return (int)i;
        }
    }

    return -1;
}


void Signature::addReturn(SharedType type, SharedExp exp)
{
    assert(exp);
    addReturn(std::make_shared<Return>(type, exp));
    //    rettype = type->clone();
}


void Signature::addReturn(SharedExp exp)
{
    // addReturn(exp->getType() ? exp->getType() : new IntegerType(), exp);
    addReturn(PointerType::get(VoidType::get()), exp);
}


void Signature::removeReturn(SharedExp e)
{
    int i = findReturn(e);

    if (i != -1) {
        for (unsigned j = i + 1; j < m_returns.size(); j++) {
            m_returns[j - 1] = m_returns[j];
        }

        m_returns.resize(m_returns.size() - 1);
    }
}


void Signature::setReturnType(size_t n, SharedType ty)
{
    if (n < m_returns.size()) {
        m_returns[n]->m_type = ty;
    }
}


SharedExp Signature::getArgumentExp(int n) const
{
    return getParamExp(n);
}


std::shared_ptr<Signature> Signature::promote(UserProc *p)
{
    // FIXME: the whole promotion idea needs a redesign...
    if (CallingConvention::Win32Signature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::Win32Signature(*this));
    }

    if (CallingConvention::StdC::PentiumSignature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::PentiumSignature(*this));
    }

    if (CallingConvention::StdC::SparcSignature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::SparcSignature(*this));
    }

    if (CallingConvention::StdC::PPCSignature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::PPCSignature(*this));
    }

    if (CallingConvention::StdC::ST20Signature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::ST20Signature(*this));
    }

    return shared_from_this();
}


std::shared_ptr<Signature> Signature::instantiate(Platform plat, CallConv cc, const QString& nam)
{
    switch (plat)
    {
    case Platform::PENTIUM:

        if (cc == CallConv::Pascal) {
            // For now, assume the only pascal calling convention pentium signatures will be Windows
            return std::make_shared<CallingConvention::Win32Signature>(nam);
        }
        else if (cc == CallConv::ThisCall) {
            return std::make_shared<CallingConvention::Win32TcSignature>(nam);
        }
        else {
            return std::make_shared<CallingConvention::StdC::PentiumSignature>(nam);
        }

    case Platform::SPARC:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        assert(cc == CallConv::C);
        return std::make_shared<CallingConvention::StdC::SparcSignature>(nam);

    case Platform::PPC:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        return std::make_shared<CallingConvention::StdC::PPCSignature>(nam);

    case Platform::ST20:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        return std::make_shared<CallingConvention::StdC::ST20Signature>(nam);

    case Platform::MIPS:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        return std::make_shared<CallingConvention::StdC::MIPSSignature>(nam);

    // insert other conventions here
    default:
        LOG_ERROR("Unknown signature: %1 %2", getConventionName(cc), getPlatformName(plat));
        return nullptr;
    }
}


Signature::~Signature()
{
}


void Signature::print(QTextStream& out, bool /*html*/) const
{
    if (isForced()) {
        out << "*forced* ";
    }

    if (!m_returns.empty()) {
        out << "{ ";
        unsigned n = 0;

        for (const std::shared_ptr<Return>& rr : m_returns) {
            out << rr->m_type->getCtype() << " " << rr->m_exp;

            if (n != m_returns.size() - 1) {
                out << ",";
            }

            out << " ";
            n++;
        }

        out << "} ";
    }
    else {
        out << "void ";
    }

    out << m_name << "(";

    for (unsigned int i = 0; i < m_params.size(); i++) {
        out << m_params[i]->getType()->getCtype() << " " << m_params[i]->getName() << " " << m_params[i]->getExp();

        if (i != m_params.size() - 1) {
            out << ", ";
        }
    }

    out << ")";
}


char *Signature::prints() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    tgt += "\n";

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void Signature::printToLog() const
{
    QString     tgt;
    QTextStream os(&tgt);

    print(os);
    LOG_MSG(tgt);
}


bool Signature::usesNewParam(UserProc * /*p*/, Instruction *stmt, bool checkreach, int& n) const
{
    QTextStream q_cerr(stderr);

    n = getNumParams() - 1;

    if (VERBOSE) {
        q_cerr << "searching ";
        stmt->printAsUse(q_cerr);
        q_cerr << '\n';
    }

    InstructionSet reachin;

    // stmt->getReachIn(reachin, 2);
    for (int i = getNumParams(); i < 10; i++) {
        if (stmt->usesExp(*getParamExp(i))) {
            bool ok = true;

            if (checkreach) {
                bool hasDef = false;

                for (Instruction *ins : reachin) {
                    Assignment *as = (Assignment *)ins;

                    if (as->isAssignment() && (*as->getLeft() == *getParamExp(i))) {
                        hasDef = true;
                        break;
                    }
                }

                if (hasDef) {
                    ok = false;
                }
            }

            if (ok) {
                n = i;
            }
        }
    }

    return n > ((int)getNumParams() - 1);
}


SharedExp Signature::getFirstArgLoc(Prog *prog) const
{
    Machine mach = prog->getMachine();

    switch (mach)
    {
    case Machine::SPARC:
        {
            CallingConvention::StdC::SparcSignature sig("");
            return sig.getArgumentExp(0);
        }

    case Machine::PENTIUM:
        {
            // CallingConvention::StdC::PentiumSignature sig("");
            // Exp* e = sig.getArgumentExp(0);
            // For now, need to work around how the above appears to be the wrong thing!
            SharedExp e = Location::memOf(Location::regOf(28));
            return e;
        }

    case Machine::ST20:
        {
            CallingConvention::StdC::ST20Signature sig("");
            return sig.getArgumentExp(0);
            // return Location::regOf(0);
        }

    default:
        LOG_FATAL("Machine %1 not handled", (int)mach);
    }

    return nullptr;
}


/*static*/ SharedExp Signature::getReturnExp2(IFileLoader *pBF)
{
    switch (pBF->getMachine())
    {
    case Machine::SPARC:
        return Location::regOf(8);

    case Machine::PENTIUM:
        return Location::regOf(24);

    case Machine::ST20:
        return Location::regOf(0);

    default:
        LOG_WARN("Machine not handled");
    }

    return nullptr;
}


void Signature::setABIdefines(Prog *prog, StatementList *defs)
{
    if (defs->size()) {
        return; // Do only once
    }

    switch (prog->getMachine())
    {
    case Machine::PENTIUM:
        defs->append(new ImplicitAssign(Location::regOf(24))); // eax
        defs->append(new ImplicitAssign(Location::regOf(25))); // ecx
        defs->append(new ImplicitAssign(Location::regOf(26))); // edx
        break;

    case Machine::SPARC:

        for (int r = 8; r <= 13; ++r) {
            defs->append(new ImplicitAssign(Location::regOf(r))); // %o0-o5
        }

        defs->append(new ImplicitAssign(Location::regOf(1)));     // %g1
        break;

    case Machine::PPC:

        for (int r = 3; r <= 12; ++r) {
            defs->append(new ImplicitAssign(Location::regOf(r))); // r3-r12
        }

        break;

    case Machine::ST20:
        defs->append(new ImplicitAssign(Location::regOf(0))); // A
        defs->append(new ImplicitAssign(Location::regOf(1))); // B
        defs->append(new ImplicitAssign(Location::regOf(2))); // C
        break;

    default:
        break;
    }
}


SharedExp Signature::getEarlyParamExp(int n, Prog *prog) const
{
    switch (prog->getMachine())
    {
    case Machine::SPARC:
        {
            CallingConvention::StdC::SparcSignature temp("");
            return temp.getParamExp(n);
        }

    case Machine::PENTIUM:
        {
            // Would we ever need Win32?
            CallingConvention::StdC::PentiumSignature temp("");
            return temp.getParamExp(n);
        }

    case Machine::ST20:
        {
            CallingConvention::StdC::ST20Signature temp("");
            return temp.getParamExp(n);
        }

    default:
        break;
    }

    assert(false); // Machine not handled
    return nullptr;
}


StatementList& Signature::getStdRetStmt(Prog *prog)
{
    // pc := m[r[28]]
    static Assign pent1ret(Terminal::get(opPC), Location::memOf(Location::regOf(28)));
    // r[28] := r[28] + 4
    static Assign pent2ret(Location::regOf(28), Binary::get(opPlus, Location::regOf(28), Const::get(4)));
    static Assign st20_1ret(Terminal::get(opPC), Location::memOf(Location::regOf(3)));
    static Assign st20_2ret(Location::regOf(3), Binary::get(opPlus, Location::regOf(3), Const::get(16)));

    switch (prog->getMachine())
    {
    case Machine::SPARC:
        break; // No adjustment to stack pointer required

    case Machine::PENTIUM:
        {
            StatementList *sl = new StatementList;
            sl->append((Instruction *)&pent1ret);
            sl->append((Instruction *)&pent2ret);
            return *sl;
        }

    case Machine::ST20:
        {
            StatementList *sl = new StatementList;
            sl->append((Instruction *)&st20_1ret);
            sl->append((Instruction *)&st20_2ret);
            return *sl;
        }

    default:
        break;
    }

    return *new StatementList;
}


int Signature::getStackRegister() const noexcept(false)
{
    LOG_VERBOSE("thowing StackRegisterNotDefinedException");

    throw StackRegisterNotDefinedException();
}


int Signature::getStackRegister(Prog *prog) noexcept(false)
{
    switch (prog->getMachine())
    {
    case Machine::SPARC:
        return 14;

    case Machine::PENTIUM:
        return 28;

    case Machine::PPC:
        return 1;

    case Machine::ST20:
        return 3;

    default:
        throw StackRegisterNotDefinedException();
    }
}


bool Signature::isStackLocal(Prog *prog, SharedExp e) const
{
    // e must be m[...]
    if (e->isSubscript()) {
        return isStackLocal(prog, e->getSubExp1());
    }

    if (!e->isMemOf()) {
        return false;
    }

    SharedExp addr = e->getSubExp1();
    return isAddrOfStackLocal(prog, addr);
}


bool Signature::isAddrOfStackLocal(Prog *prog, const SharedExp& e) const
{
    OPER op = e->getOper();

    if (op == opAddrOf) {
        return isStackLocal(prog, e->getSubExp1());
    }

    // e must be sp -/+ K or just sp
    static SharedExp sp = Location::regOf(getStackRegister(prog));

    if ((op != opMinus) && (op != opPlus)) {
        // Matches if e is sp or sp{0} or sp{-}
        return(*e == *sp ||
               (e->isSubscript() && e->access<RefExp>()->isImplicitDef() && *e->getSubExp1() == *sp));
    }

    if ((op == opMinus) && !isLocalOffsetNegative()) {
        return false;
    }

    if ((op == opPlus) && !isLocalOffsetPositive()) {
        return false;
    }

    SharedExp sub1 = e->getSubExp1();
    SharedExp sub2 = e->getSubExp2();

    // e must be <sub1> +- K
    if (!sub2->isIntConst()) {
        return false;
    }

    // first operand must be sp or sp{0} or sp{-}
    if (sub1->isSubscript()) {
        if (!sub1->access<RefExp>()->isImplicitDef()) {
            return false;
        }

        sub1 = sub1->getSubExp1();
    }

    return *sub1 == *sp;
}


bool CallingConvention::StdC::SparcSignature::isAddrOfStackLocal(Prog *prog, const SharedExp& e) const
{
    OPER op = e->getOper();

    if (op == opAddrOf) {
        return isStackLocal(prog, e->getSubExp1());
    }

    // e must be sp -/+ K or just sp
    static SharedExp sp = Location::regOf(14);

    if ((op != opMinus) && (op != opPlus)) {
        // Matches if e is sp or sp{0} or sp{-}
        return(*e == *sp ||
               (e->isSubscript() && e->access<RefExp>()->isImplicitDef() && *e->getSubExp1() == *sp));
    }

    SharedExp sub1 = e->getSubExp1();
    SharedExp sub2 = e->getSubExp2();

    // e must be <sub1> +- K
    if (!sub2->isIntConst()) {
        return false;
    }

    // first operand must be sp or sp{0} or sp{-}
    if (sub1->isSubscript()) {
        if (!sub1->access<RefExp>()->isImplicitDef()) {
            return false;
        }

        sub1 = sub1->getSubExp1();
    }

    if (!(*sub1 == *sp)) {
        return false;
    }

    // SPARC specific test: K must be < 92; else it is a parameter
    int K = sub2->access<Const>()->getInt();
    return K < 92;
}


bool Parameter::operator==(Parameter& other) const
{
    if (!(*m_type == *other.m_type)) {
        return false;
    }

    // Do we really care about a parameter's name?
    if (!(m_name == other.m_name)) {
        return false;
    }

    if (!(*m_exp == *other.m_exp)) {
        return false;
    }

    return true;
}


bool Signature::isOpCompatStackLocal(OPER op) const
{
    if (op == opMinus) {
        return isLocalOffsetNegative();
    }

    if (op == opPlus) {
        return isLocalOffsetPositive();
    }

    return false;
}


bool Signature::returnCompare(Assignment& a, Assignment& b) const
{
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}


bool Signature::argumentCompare(Assignment& a, Assignment& b) const
{
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}


bool CallingConvention::StdC::PentiumSignature::returnCompare(Assignment& a, Assignment& b) const
{
    SharedExp la = a.getLeft();
    SharedExp lb = b.getLeft();

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


static Unary spPlus64(opMemOf, Binary::get(opPlus, Location::regOf(14), Const::get(64)));

bool CallingConvention::StdC::SparcSignature::returnCompare(Assignment& a, Assignment& b) const
{
    SharedExp la = a.getLeft();
    SharedExp lb = b.getLeft();

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


// From m[sp +- K] return K (or -K for subtract). sp could be subscripted with {-}
// Helper function for the below
int stackOffset(SharedExp e, int sp)
{
    int ret = 0;

    if (e->isMemOf()) {
        SharedExp sub = e->getSubExp1();
        OPER      op  = sub->getOper();

        if ((op == opPlus) || (op == opMinus)) {
            SharedExp op1 = sub->getSubExp1();

            if (op1->isSubscript()) {
                op1 = op1->getSubExp1();
            }

            if (op1->isRegN(sp)) {
                SharedExp op2 = sub->getSubExp2();

                if (op2->isIntConst()) {
                    ret = op2->access<Const>()->getInt();
                }

                if (op == opMinus) {
                    ret = -ret;
                }
            }
        }
    }

    return ret;
}


bool CallingConvention::StdC::PentiumSignature::argumentCompare(Assignment& a, Assignment& b) const
{
    SharedExp la = a.getLeft();
    SharedExp lb = b.getLeft();
    int       ma = stackOffset(la, 28);
    int       mb = stackOffset(lb, 28);

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


bool CallingConvention::StdC::SparcSignature::argumentCompare(Assignment& a, Assignment& b) const
{
    SharedExp la = a.getLeft();
    SharedExp lb = b.getLeft();
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

    int ma = stackOffset(la, 30);
    int mb = stackOffset(lb, 30);

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


std::shared_ptr<Return> Return::clone() const
{
    return std::make_shared<Return>(m_type->clone(), SharedExp(m_exp->clone()));
}


bool Return::operator==(Return& other) const
{
    if (!(*m_type == *other.m_type)) {
        return false;
    }

    if (!(*m_exp == *other.m_exp)) {
        return false;
    }

    return true;
}


SharedType Signature::getTypeFor(SharedExp e) const
{
    size_t n = m_returns.size();

    for (size_t i = 0; i < n; ++i) {
        if (*m_returns[i]->m_exp == *e) {
            return m_returns[i]->m_type;
        }
    }

    return nullptr;
}
