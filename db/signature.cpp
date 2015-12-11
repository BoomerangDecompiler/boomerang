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

#include "signature.h"

#include "type.h"
#include "signature.h"
#include "exp.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "util.h"
#include "cfg.h"
#include "proc.h"
#include "boomerang.h"
#include "log.h"
#include "managed.h"

#include <QtCore/QDebug>
#include <cassert>
#include <string>
#include <cstring>
#include <sstream>
extern char debug_buffer[]; // For prints()

QString Signature::platformName(platform plat) {
    switch (plat) {
    case PLAT_PENTIUM:
        return "pentium";
    case PLAT_SPARC:
        return "sparc";
    case PLAT_M68K:
        return "m68k";
    case PLAT_PARISC:
        return "parisc";
    case PLAT_PPC:
        return "ppc";
    case PLAT_MIPS:
        return "mips";
    case PLAT_ST20:
        return "st20";
    default:
        return "???";
    }
}

QString Signature::conventionName(callconv cc) {
    switch (cc) {
    case CONV_C:
        return "stdc";
    case CONV_PASCAL:
        return "pascal";
    case CONV_THISCALL:
        return "thiscall";
    default:
        return "??";
    }
}

namespace CallingConvention {

class Win32Signature : public Signature {
    // Win32Signature is for non-thiscall signatures: all parameters pushed
  public:
    Win32Signature(const QString &nam);
    Win32Signature(Signature &old);
    virtual ~Win32Signature() {}
    virtual Signature *clone() override;
    virtual bool operator==(Signature &other) override;
    static bool qualified(UserProc *p, Signature &candidate);

    virtual void addReturn(SharedType type, Exp *e = nullptr) override;
    virtual void addParameter(SharedType type, const QString &nam = QString::null, Exp *e = nullptr,
                              const QString &boundMax = "") override;
    virtual Exp *getArgumentExp(int n) override;

    virtual Signature *promote(UserProc *) override;
    virtual Exp *getStackWildcard() override;
    virtual int getStackRegister() noexcept(false) override { return 28; }
    virtual Exp *getProven(Exp *left) override;
    virtual bool isPreserved(Exp *e) override;                    // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList *defs) override; // Set list of locations def'd by library calls

    virtual bool isPromoted() override { return true; }
    virtual platform getPlatform() override { return PLAT_PENTIUM; }
    virtual callconv getConvention() override { return CONV_PASCAL; }
}; // class Win32Signature

class Win32TcSignature : public Win32Signature {
    // Win32TcSignature is for "thiscall" signatures, i.e. those that have register ecx as the first parameter
    // Only needs to override a few member functions; the rest can inherit from Win32Signature
  public:
    Win32TcSignature(const QString &nam);
    Win32TcSignature(Signature &old);
    virtual Exp *getArgumentExp(int n) override;
    virtual Exp *getProven(Exp *left) override;
    virtual Signature *clone() override;
    virtual platform getPlatform() override { return PLAT_PENTIUM; }
    virtual callconv getConvention() override { return CONV_THISCALL; }
}; // Class Win32TcSignature

namespace StdC {
class PentiumSignature : public Signature {
  public:
    PentiumSignature(const QString &nam);
    PentiumSignature(Signature &old);
    virtual ~PentiumSignature() {}
    virtual Signature *clone() override;
    virtual bool operator==(Signature &other) override;
    static bool qualified(UserProc *p, Signature &);

    virtual void addReturn(SharedType type, Exp *e = nullptr) override;
    virtual void addParameter(SharedType type, const QString &nam = QString::null, Exp *e = nullptr,
                              const QString &boundMax = "") override;
    virtual Exp *getArgumentExp(int n) override;

    virtual Signature *promote(UserProc *) override;
    virtual Exp *getStackWildcard() override;
    virtual int getStackRegister() noexcept(false) override { return 28; }
    virtual Exp *getProven(Exp *left) override;
    virtual bool isPreserved(Exp *e) override;                    // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList *defs) override; // Set list of locations def'd by library calls
    virtual bool isPromoted() override { return true; }
    virtual platform getPlatform() override { return PLAT_PENTIUM; }
    virtual callconv getConvention() override { return CONV_C; }
    virtual bool returnCompare(Assignment &a, Assignment &b) override;
    virtual bool argumentCompare(Assignment &a, Assignment &b) override;
}; // class PentiumSignature

class SparcSignature : public Signature {
  public:
    SparcSignature(const QString &nam);
    SparcSignature(Signature &old);
    virtual ~SparcSignature() {}
    virtual Signature *clone() override;
    virtual bool operator==(Signature &other) override;
    static bool qualified(UserProc *p, Signature &);

    virtual void addReturn(SharedType type, Exp *e = nullptr) override;
    virtual void addParameter(SharedType type, const QString &nam = QString::null, Exp *e = nullptr,
                              const QString &boundMax = "") override;
    virtual Exp *getArgumentExp(int n) override;

    virtual Signature *promote(UserProc *) override;
    virtual Exp *getStackWildcard() override;
    virtual int getStackRegister() noexcept(false) override { return 14; }
    virtual Exp *getProven(Exp *left) override;
    virtual bool isPreserved(Exp *e) override;                    // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList *defs) override; // Set list of locations def'd by library calls
    // Stack offsets can be negative (inherited) or positive:
    virtual bool isLocalOffsetPositive() override { return true; }
    // An override for testing locals
    virtual bool isAddrOfStackLocal(Prog *prog, Exp *e) override;
    virtual bool isPromoted() override { return true; }
    virtual platform getPlatform() override { return PLAT_SPARC; }
    virtual callconv getConvention() override { return CONV_C; }
    virtual bool returnCompare(Assignment &a, Assignment &b) override;
    virtual bool argumentCompare(Assignment &a, Assignment &b) override;
}; // class SparcSignature

class SparcLibSignature : public SparcSignature {
  public:
    SparcLibSignature(const QString &nam) : SparcSignature(nam) {}
    SparcLibSignature(Signature &old);
    virtual Signature *clone() override;
    virtual Exp *getProven(Exp *left) override;
}; // class SparcLibSignature

class PPCSignature : public Signature {
  public:
    PPCSignature(const QString &name);
    PPCSignature(Signature &old);
    virtual ~PPCSignature() {}
    virtual Signature *clone() override;
    static bool qualified(UserProc *p, Signature &);
    virtual void addReturn(SharedType type, Exp *e = nullptr) override;
    virtual Exp *getArgumentExp(int n) override;
    virtual void addParameter(SharedType type, const QString &nam = QString::null, Exp *e = nullptr,
                              const QString &boundMax = "") override;
    virtual Exp *getStackWildcard() override;
    virtual int getStackRegister() noexcept(false) override { return 1; }
    virtual Exp *getProven(Exp *left) override;
    virtual bool isPreserved(Exp *e) override;                    // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList *defs) override; // Set list of locations def'd by library calls
    virtual bool isLocalOffsetPositive() override { return true; }
    virtual bool isPromoted() override { return true; }
    virtual platform getPlatform() override { return PLAT_PPC; }
    virtual callconv getConvention() override { return CONV_C; }
    Signature *promote(UserProc * /*p*/) override {
        // No promotions from here up, obvious idea would be c++ name mangling
        return this;
    }
};
class MIPSSignature : public Signature {
  public:
    MIPSSignature(const QString &name);
    MIPSSignature(Signature &old);
    virtual ~MIPSSignature() {}
    virtual Signature *clone() override;
    static bool qualified(UserProc *p, Signature &);
    virtual void addReturn(SharedType type, Exp *e = nullptr) override;
    virtual Exp *getArgumentExp(int n) override;
    virtual void addParameter(SharedType type, const QString &nam = QString::null, Exp *e = nullptr,
                              const QString &boundMax = "") override;
    virtual Exp *getStackWildcard() override;
    virtual int getStackRegister() noexcept(false) override { return 29; }
    virtual Exp *getProven(Exp *left) override;
    virtual bool isPreserved(Exp *e) override;                    // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList *defs) override; // Set list of locations def'd by library calls
    virtual bool isLocalOffsetPositive() override { return true; }
    virtual bool isPromoted() override { return true; }
    virtual platform getPlatform() override { return PLAT_MIPS; }
    virtual callconv getConvention() override { return CONV_C; }
};
class ST20Signature : public Signature {
  public:
    ST20Signature(const QString &name);
    ST20Signature(Signature &old);
    virtual ~ST20Signature() {}
    Signature *clone() override;
    virtual bool operator==(Signature &other) override;
    static bool qualified(UserProc *p, Signature &);

    virtual void addReturn(SharedType type, Exp *e = nullptr) override;
    void addParameter(SharedType type, const QString &nam = QString::null, Exp *e = nullptr,
                      const QString &boundMax = "") override;
    Exp *getArgumentExp(int n) override;

    virtual Signature *promote(UserProc *) override;
    virtual Exp *getStackWildcard() override;
    virtual int getStackRegister() noexcept(false) override { return 3; }
    virtual Exp *getProven(Exp *left) override;
    virtual bool isPromoted() override { return true; }
    // virtual bool isLocalOffsetPositive() {return true;}
    virtual platform getPlatform() override { return PLAT_ST20; }
    virtual callconv getConvention() override { return CONV_C; }
};
} // namespace StdC
} // namespace CallingConvention

CallingConvention::Win32Signature::Win32Signature(const QString &nam) : Signature(nam) {
    Signature::addReturn(Location::regOf(28));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                Location::regOf(28), nullptr);
}

CallingConvention::Win32Signature::Win32Signature(Signature &old) : Signature(old) {}

CallingConvention::Win32TcSignature::Win32TcSignature(const QString &nam) : Win32Signature(nam) {
    Signature::addReturn(Location::regOf(28));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                Location::regOf(28), nullptr);
}

CallingConvention::Win32TcSignature::Win32TcSignature(Signature &old) : Win32Signature(old) {}

static void cloneVec(std::vector<Parameter *> &from, std::vector<Parameter *> &to) {
    unsigned n = from.size();
    to.resize(n);
    for (unsigned i = 0; i < n; i++)
        to[i] = from[i]->clone();
}

static void cloneVec(Returns &from, Returns &to) {
    unsigned n = from.size();
    to.resize(n);
    for (unsigned i = 0; i < n; i++)
        to[i] = from[i]->clone();
}

Parameter::~Parameter() {
    delete exp;
}
Parameter *Parameter::clone() { return new Parameter(type->clone(), m_name, exp->clone(), boundMax); }

void Parameter::setBoundMax(const QString &nam) {
    boundMax = nam;
}

Signature *CallingConvention::Win32Signature::clone() {
    Win32Signature *n = new Win32Signature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    if (preferedReturn)
        n->preferedReturn = preferedReturn->clone();
    else
        n->preferedReturn = nullptr;
    n->preferedParams = preferedParams;
    return n;
}

Signature *CallingConvention::Win32TcSignature::clone() {
    Win32TcSignature *n = new Win32TcSignature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    if (preferedReturn)
        n->preferedReturn = preferedReturn->clone();
    else
        n->preferedReturn = nullptr;
    n->preferedParams = preferedParams;
    return n;
}

bool CallingConvention::Win32Signature::operator==(Signature &other) { return Signature::operator==(other); }

static Exp *savedReturnLocation = Location::memOf(Location::regOf(28));
static Exp *stackPlusFour = Binary::get(opPlus, Location::regOf(28), new Const(4));

bool CallingConvention::Win32Signature::qualified(UserProc *p, Signature & /*candidate*/) {
    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_PENTIUM || !p->getProg()->isWin32())
        return false;

    if (VERBOSE)
        LOG << "consider promotion to stdc win32 signature for " << p->getName() << "\n";

    bool gotcorrectret1, gotcorrectret2 = false;
    Exp *provenPC = p->getProven(new Terminal(opPC));
    gotcorrectret1 = provenPC && (*provenPC == *savedReturnLocation);
    if (gotcorrectret1) {
        if (VERBOSE)
            LOG << "got pc = m[r[28]]\n";
        Exp *provenSP = p->getProven(Location::regOf(28));
        gotcorrectret2 = provenSP && *provenSP == *stackPlusFour;
        if (gotcorrectret2 && VERBOSE)
            LOG << "got r[28] = r[28] + 4\n";
    }
    if (VERBOSE)
        LOG << "qualified: " << (gotcorrectret1 && gotcorrectret2) << "\n";
    return gotcorrectret1 && gotcorrectret2;
}

void CallingConvention::Win32Signature::addReturn(SharedType type, Exp *e) {
    if (type->isVoid())
        return;
    if (e == nullptr) {
        if (type->isFloat())
            e = Location::regOf(32);
        else
            e = Location::regOf(24);
    }
    Signature::addReturn(type, e);
}

void CallingConvention::Win32Signature::addParameter(SharedType type, const QString &nam, Exp *e,
                                                     const QString &boundMax) {
    if (e == nullptr) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e, boundMax);
}

Exp *CallingConvention::Win32Signature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *esp = Location::regOf(28);
    if (params.size() != 0 && *params[0]->getExp() == *esp)
        n--;
    Exp *e = Location::memOf(Binary::get(opPlus, esp, new Const((n + 1) * 4)));
    return e;
}

Exp *CallingConvention::Win32TcSignature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *esp = Location::regOf(28);
    if (!params.empty() && *params[0]->getExp() == *esp)
        n--;
    if (n == 0) {
        delete esp;
        // It's the first parameter, register ecx
        return Location::regOf(25);
    }
    // Else, it is m[esp+4n)]
    Exp *e = Location::memOf(Binary::get(opPlus, esp, new Const(n * 4)));
    return e;
}

Signature *CallingConvention::Win32Signature::promote(UserProc * /*p*/) {
    // no promotions from win32 signature up, yet.
    // a possible thing to investigate would be COM objects
    return this;
}

Exp *CallingConvention::Win32Signature::getStackWildcard() {
    // Note: m[esp + -8] is simplified to m[esp - 8] now
    return Location::memOf(Binary::get(opMinus, Location::regOf(28), new Terminal(opWild)));
}

Exp *CallingConvention::Win32Signature::getProven(Exp *left) {
    int nparams = params.size();
    if (nparams > 0 && *params[0]->getExp() == *Location::regOf(28)) {
        nparams--;
    }
    if (left->isRegOfK()) {
        switch (((Const *)left->getSubExp1())->getInt()) {
        case 28: // esp
            // Note: assumes callee pop... not true for cdecl functions!
            return Binary::get(opPlus, Location::regOf(28), new Const(4 + nparams * 4));
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

bool CallingConvention::Win32Signature::isPreserved(Exp *e) {
    if (e->isRegOfK()) {
        switch (((Const *)e->getSubExp1())->getInt()) {
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

// Return a list of locations defined by library calls
void CallingConvention::Win32Signature::setLibraryDefines(StatementList *defs) {
    if (defs->size())
        return;                     // Do only once
    auto r24 = Location::regOf(24); // eax
    SharedType ty = SizeType::get(32);
    if (returns.size() > 1) { // Ugh - note the stack pointer is the first return still
        ty = returns[1]->type;
#if 0 // ADHOC TA
        if (ty->isFloat()) {
            Location* r32 = Location::regOf(32);                // Top of FP stack
            r32->setType(ty);
        } else
            r24->setType(ty);                                    // All others return in r24 (check!)
#endif
    }
    defs->append(new ImplicitAssign(ty, r24));             // eax
    defs->append(new ImplicitAssign(Location::regOf(25))); // ecx
    defs->append(new ImplicitAssign(Location::regOf(26))); // edx
    defs->append(new ImplicitAssign(Location::regOf(28))); // esp
}

Exp *CallingConvention::Win32TcSignature::getProven(Exp *left) {
    if (left->isRegOfK()) {
        if (((Const *)left->getSubExp1())->getInt() == 28) {
            int nparams = params.size();
            if (nparams > 0 && *params[0]->getExp() == *Location::regOf(28)) {
                nparams--;
            }
            // r28 += 4 + nparams*4 - 4        (-4 because ecx is register param)
            return Binary::get(opPlus, Location::regOf(28), new Const(4 + nparams * 4 - 4));
        }
    }
    // Else same as for standard Win32 signature
    return Win32Signature::getProven(left);
}

CallingConvention::StdC::PentiumSignature::PentiumSignature(const QString &nam) : Signature(nam) {
    Signature::addReturn(Location::regOf(28));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "esp",
    //                                 Location::regOf(28), nullptr);
}

CallingConvention::StdC::PentiumSignature::PentiumSignature(Signature &old) : Signature(old) {}

Signature *CallingConvention::StdC::PentiumSignature::clone() {
    PentiumSignature *n = new PentiumSignature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    if (preferedReturn)
        n->preferedReturn = preferedReturn->clone();
    else
        n->preferedReturn = nullptr;
    n->preferedParams = preferedParams;
    n->unknown = unknown;
    return n;
}

bool CallingConvention::StdC::PentiumSignature::operator==(Signature &other) { return Signature::operator==(other); }

// FIXME: This needs changing. Would like to check that pc=pc and sp=sp
// (or maybe sp=sp+4) for qualifying procs. Need work to get there
bool CallingConvention::StdC::PentiumSignature::qualified(UserProc *p, Signature & /*candidate*/) {
    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_PENTIUM)
        return false;

    LOG_VERBOSE(1) << "consider promotion to stdc pentium signature for " << p->getName() << "\n";

#if 1
    LOG_VERBOSE(1) << "qualified: always true\n";
    return true; // For now, always pass
#else
    bool gotcorrectret1 = false;
    bool gotcorrectret2 = false;
    StatementList internal;
    // p->getInternalStatements(internal);
    internal.append(*p->getCFG()->getReachExit());
    StmtListIter it;
    for (Statement *s = internal.getFirst(it); s; s = internal.getNext(it)) {
        Assign *e = dynamic_cast<Assign *>(s);
        if (e == nullptr)
            continue;
        if (e->getLeft()->getOper() == opPC) {
            if (e->getRight()->isMemOf() && e->getRight()->getSubExp1()->isRegOfN(28)) {
                if (VERBOSE)
                    LOG_STREAM() << "got pc = m[r[28]]" << '\n';
                gotcorrectret1 = true;
            }
        } else if (e->getLeft()->isRegOfK() && ((Const *)e->getLeft()->getSubExp1())->getInt() == 28) {
            if (e->getRight()->getOper() == opPlus && e->getRight()->getSubExp1()->isRegOfN(28) &&
                e->getRight()->getSubExp2()->isIntConst() && ((Const *)e->getRight()->getSubExp2())->getInt() == 4) {
                if (VERBOSE)
                    LOG_STREAM() << "got r[28] = r[28] + 4" << '\n';
                gotcorrectret2 = true;
            }
        }
    }
    if (VERBOSE)
        LOG << "promotion: " << gotcorrectret1 &&gotcorrectret2 << "\n";
    return gotcorrectret1 && gotcorrectret2;
#endif
}

void CallingConvention::StdC::PentiumSignature::addReturn(SharedType type, Exp *e) {
    if (type->isVoid())
        return;
    if (e == nullptr) {
        if (type->isFloat())
            e = Location::regOf(32);
        else
            e = Location::regOf(24);
    }
    Signature::addReturn(type, e);
}

void CallingConvention::StdC::PentiumSignature::addParameter(SharedType type, const QString &nam, Exp *e,
                                                             const QString &boundMax) {
    if (e == nullptr) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e, boundMax);
}

Exp *CallingConvention::StdC::PentiumSignature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *esp = Location::regOf(28);
    if (params.size() != 0 && *params[0]->getExp() == *esp)
        n--;
    Exp *e = Location::memOf(Binary::get(opPlus, esp, new Const((n + 1) * 4)));
    return e;
}

Signature *CallingConvention::StdC::PentiumSignature::promote(UserProc * /*p*/) {
    // No promotions from here up, obvious idea would be c++ name mangling
    return this;
}

Exp *CallingConvention::StdC::PentiumSignature::getStackWildcard() {
    // Note: m[esp + -8] is simplified to m[esp - 8] now
    return Location::memOf(Binary::get(opMinus, Location::regOf(28), new Terminal(opWild)));
}

Exp *CallingConvention::StdC::PentiumSignature::getProven(Exp *left) {
    if (left->isRegOfK()) {
        int r = ((Const *)left->getSubExp1())->getInt();
        switch (r) {
        case 28:                                                           // esp
            return Binary::get(opPlus, Location::regOf(28), new Const(4)); // esp+4
        case 29:
        case 30:
        case 31:
        case 27: // ebp, esi, edi, ebx
            return Location::regOf(r);
        }
    }
    return nullptr;
}

bool CallingConvention::StdC::PentiumSignature::isPreserved(Exp *e) {
    if (e->isRegOfK()) {
        switch (((Const *)e->getSubExp1())->getInt()) {
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

// Return a list of locations defined by library calls
void CallingConvention::StdC::PentiumSignature::setLibraryDefines(StatementList *defs) {
    if (defs->size())
        return;                     // Do only once
    auto r24 = Location::regOf(24); // eax
    SharedType ty = SizeType::get(32);
    if (returns.size() > 1) { // Ugh - note the stack pointer is the first return still
        ty = returns[1]->type;
#if 0 // ADHOC TA
        if (ty->isFloat()) {
            Location* r32 = Location::regOf(32);            // Top of FP stack
            r32->setType(ty);
        } else
            r24->setType(ty);                                    // All others return in r24 (check!)
#endif
    }
    defs->append(new ImplicitAssign(ty, r24));             // eax
    defs->append(new ImplicitAssign(Location::regOf(25))); // ecx
    defs->append(new ImplicitAssign(Location::regOf(26))); // edx
    defs->append(new ImplicitAssign(Location::regOf(28))); // esp
}

CallingConvention::StdC::PPCSignature::PPCSignature(const QString &nam) : Signature(nam) {
    Signature::addReturn(Location::regOf(1));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "r1",
    //                                 Location::regOf(1), nullptr);
    // FIXME: Should also add m[r1+4] as an implicit parameter? Holds return address
}

CallingConvention::StdC::PPCSignature::PPCSignature(Signature &old) : Signature(old) {}

Signature *CallingConvention::StdC::PPCSignature::clone() {
    PPCSignature *n = new PPCSignature(name);
    cloneVec(params, n->params);
    // n->implicitParams = implicitParams;
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    if (preferedReturn)
        n->preferedReturn = preferedReturn->clone();
    else
        n->preferedReturn = nullptr;
    n->preferedParams = preferedParams;
    n->unknown = unknown;
    return n;
}

Exp *CallingConvention::StdC::PPCSignature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *e;
    if (n >= 8) {
        // PPCs pass the ninth and subsequent parameters at m[%r1+8],
        // m[%r1+12], etc.
        e = Location::memOf(Binary::get(opPlus, Location::regOf(1), new Const(8 + (n - 8) * 4)));
    } else
        e = Location::regOf(3 + n);
    return e;
}

void CallingConvention::StdC::PPCSignature::addReturn(SharedType type, Exp *e) {
    if (type->isVoid())
        return;
    if (e == nullptr) {
        e = Location::regOf(3);
    }
    Signature::addReturn(type, e);
}

void CallingConvention::StdC::PPCSignature::addParameter(SharedType type, const QString &nam,
                                                         Exp *e, const QString &boundMax) {
    if (e == nullptr) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e, boundMax);
}

Exp *CallingConvention::StdC::PPCSignature::getStackWildcard() {
    // m[r1 - WILD]
    return Location::memOf(Binary::get(opMinus, Location::regOf(1), new Terminal(opWild)));
}

Exp *CallingConvention::StdC::PPCSignature::getProven(Exp *left) {
    if (left->isRegOfK()) {
        int r = ((Const *)((Location *)left)->getSubExp1())->getInt();
        switch (r) {
        case 1: // stack
            return left;
        }
    }
    return nullptr;
}

bool CallingConvention::StdC::PPCSignature::isPreserved(Exp *e) {
    if (e->isRegOfK()) {
        int r = ((Const *)e->getSubExp1())->getInt();
        return r == 1;
    }
    return false;
}

// Return a list of locations defined by library calls
void CallingConvention::StdC::PPCSignature::setLibraryDefines(StatementList *defs) {
    if (defs->size())
        return; // Do only once
    for (int r = 3; r <= 12; ++r)
        defs->append(new ImplicitAssign(Location::regOf(r))); // Registers 3-12 are volatile (caller save)
}

/// ST20 signatures

CallingConvention::StdC::ST20Signature::ST20Signature(const QString &nam) : Signature(nam) {
    Signature::addReturn(Location::regOf(3));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "sp", Location::regOf(3), nullptr);
    // FIXME: Should also add m[sp+0] as an implicit parameter? Holds return address
}

CallingConvention::StdC::ST20Signature::ST20Signature(Signature &old) : Signature(old) {}

Signature *CallingConvention::StdC::ST20Signature::clone() {
    ST20Signature *n = new ST20Signature(name);
    n->params = params;
    n->returns = returns;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    n->preferedName = preferedName;
    n->preferedReturn = preferedReturn;
    n->preferedParams = preferedParams;
    n->unknown = unknown;
    return n;
}

bool CallingConvention::StdC::ST20Signature::operator==(Signature &other) { return Signature::operator==(other); }

Exp *CallingConvention::StdC::ST20Signature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    // m[%sp+4], etc.
    Exp *sp = Location::regOf(3);
    if (params.size() != 0 && *params[0]->getExp() == *sp)
        n--;
    Exp *e = Location::memOf(Binary::get(opPlus, sp, new Const((n + 1) * 4)));
    return e;
}

void CallingConvention::StdC::ST20Signature::addReturn(SharedType type, Exp *e) {
    if (type->isVoid())
        return;
    if (e == nullptr) {
        e = Location::regOf(0);
    }
    Signature::addReturn(type, e);
}

Signature *CallingConvention::StdC::ST20Signature::promote(UserProc * /*p*/) {
    // No promotions from here up, obvious idea would be c++ name mangling
    return this;
}

void CallingConvention::StdC::ST20Signature::addParameter(SharedType type, const QString &nam,
                                                          Exp *e, const QString &boundMax) {
    if (e == nullptr) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e, boundMax);
}

Exp *CallingConvention::StdC::ST20Signature::getStackWildcard() {
    // m[r1 - WILD]
    return Location::memOf(Binary::get(opMinus, Location::regOf(3), new Terminal(opWild)));
}

#if 1
Exp *CallingConvention::StdC::ST20Signature::getProven(Exp *left) {
    if (left->isRegOfK()) {
        int r = ((Const *)left->getSubExp1())->getInt();
        switch (r) {
        case 3:
            // return Binary::get(opPlus, Location::regOf(3), new Const(4));
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
#else
Exp *CallingConvention::StdC::ST20Signature::getProven(Exp *left) {
    if (left->isRegOfK()) {
        int r = ((Const *)((Location *)left)->getSubExp1())->getInt();
        switch (r) {
        case 3: // stack
            return left;
        }
    }
    return nullptr;
}
#endif

bool CallingConvention::StdC::ST20Signature::qualified(UserProc *p, Signature & /*candidate*/) {
    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_ST20)
        return false;

    if (VERBOSE)
        LOG << "consider promotion to stdc st20 signature for " << p->getName() << "\n";

    return true;
}

/*
bool CallingConvention::StdC::PPCSignature::isAddrOfStackLocal(Prog* prog, Exp* e) {
    LOG << "doing PPC specific check on " << e << "\n";
    // special case for m[r1{-} + 4] which is used to store the return address in non-leaf procs.
    if (e->getOper() == opPlus && e->getSubExp1()->isSubscript() &&
        ((RefExp*)(e->getSubExp1()))->isImplicitDef() && e->getSubExp1()->getSubExp1()->isRegOfK() &&
        ((Const*)e->getSubExp1()->getSubExp1()->getSubExp1())->getInt() == 1 && e->getSubExp2()->isIntConst() &&
        ((Const*)e->getSubExp2())->getInt() == 4)
        return true;
    return Signature::isAddrOfStackLocal(prog, e);
}
*/

CallingConvention::StdC::SparcSignature::SparcSignature(const QString &nam) : Signature(nam) {
    Signature::addReturn(Location::regOf(14));
    // Signature::addImplicitParameter(PointerType::get(new IntegerType()), "sp",
    //                                Location::regOf(14), nullptr);
}

CallingConvention::StdC::SparcSignature::SparcSignature(Signature &old) : Signature(old) {}

Signature *CallingConvention::StdC::SparcSignature::clone() {
    SparcSignature *n = new SparcSignature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    if (preferedReturn)
        n->preferedReturn = preferedReturn->clone();
    else
        n->preferedReturn = nullptr;
    n->preferedParams = preferedParams;
    n->unknown = unknown;
    return n;
}

Signature *CallingConvention::StdC::SparcLibSignature::clone() {
    SparcLibSignature *n = new SparcLibSignature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    if (preferedReturn)
        n->preferedReturn = preferedReturn->clone();
    else
        n->preferedReturn = nullptr;
    n->preferedParams = preferedParams;
    return n;
}

bool CallingConvention::StdC::SparcSignature::operator==(Signature &other) { return Signature::operator==(other); }

bool CallingConvention::StdC::SparcSignature::qualified(UserProc *p, Signature & /*candidate*/) {
    if (VERBOSE)
        LOG << "consider promotion to stdc sparc signature for " << p->getName() << "\n";

    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_SPARC)
        return false;

    if (VERBOSE)
        LOG << "Promoted to StdC::SparcSignature\n";

    return true;
}

bool CallingConvention::StdC::PPCSignature::qualified(UserProc *p, Signature & /*candidate*/) {
    if (VERBOSE)
        LOG << "consider promotion to stdc PPC signature for " << p->getName() << "\n";

    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_PPC)
        return false;

    if (VERBOSE)
        LOG << "Promoted to StdC::PPCSignature (always qualifies)\n";

    return true;
}

CallingConvention::StdC::MIPSSignature::MIPSSignature(const QString &name) : Signature(name) {
    Signature::addReturn(Location::regOf(2));
}

Signature *CallingConvention::StdC::MIPSSignature::clone()
{
    MIPSSignature *n = new MIPSSignature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    if (preferedReturn)
        n->preferedReturn = preferedReturn->clone();
    else
        n->preferedReturn = nullptr;
    n->preferedParams = preferedParams;
    n->unknown = unknown;
    return n;
}

bool CallingConvention::StdC::MIPSSignature::qualified(UserProc *p, Signature & /*candidate*/) {
    if (VERBOSE)
        LOG << "consider promotion to stdc MIPS signature for " << p->getName() << "\n";

    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_MIPS)
        return false;

    if (VERBOSE)
        LOG << "Promoted to StdC::MIPSSignature (always qualifies)\n";

    return true;
}

void CallingConvention::StdC::MIPSSignature::addReturn(SharedType type, Exp *e)
{
    if (type->isVoid())
        return;
    if (e == nullptr) {
        if(type->isInteger() || type->isPointer())
            e = Location::regOf(2); // register $2
        else if (type->isFloat())
            e = Location::regOf(32); // register $f0
        else
            e = Location::regOf(2); // register $2
    }
    Signature::addReturn(type, e);
}

Exp *CallingConvention::StdC::MIPSSignature::getArgumentExp(int n)
{
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *e;
    if (n >= 4) {
        // MIPS abi - pass the 4th and subsequent parameters at m[%sp+home_locations],
        // theo sp +0 .. home_locations contains a 'shadow' set of locations for first parameters
        // m[%esp+home_locations], etc.
        //
        e = Location::memOf(Binary::get(opPlus,
                                        Location::regOf(29), // %o6 == %sp
                                        new Const(4*4 + (n - 4) * 4)));
    } else
        e = Location::regOf((int)(8 + n));
    return e;

}

void CallingConvention::StdC::MIPSSignature::addParameter(SharedType type, const QString &nam, Exp *e, const QString &boundMax)
{
    if (e == nullptr) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e, boundMax);
}

Exp *CallingConvention::StdC::MIPSSignature::getStackWildcard()
{
    // m[%sp - WILD]
    return Location::memOf(Binary::get(opMinus, Location::regOf(getStackRegister()), new Terminal(opWild)));
}

Exp *CallingConvention::StdC::MIPSSignature::getProven(Exp *left)
{
    if (left->isRegOfK()) {
        int r = ((Const *)((Location *)left)->getSubExp1())->getInt();
        if(r==getStackRegister())
            return left;
    }
    return nullptr;
}

bool CallingConvention::StdC::MIPSSignature::isPreserved(Exp *e)
{
    if (e->isRegOfK()) {
        int r = ((Const *)e->getSubExp1())->getInt();
        return r == getStackRegister();
    }
    return false;
}

    // Return a list of locations defined by library calls
void CallingConvention::StdC::MIPSSignature::setLibraryDefines(StatementList *defs)
{
    if (defs->size())
        return; // Do only once
    for (int r = 16; r <= 23; ++r)
        defs->append(new ImplicitAssign(Location::regOf(r))); // Registers 16-23 are volatile (caller save)
    defs->append(new ImplicitAssign(Location::regOf(30)));
}

void CallingConvention::StdC::SparcSignature::addReturn(SharedType type, Exp *e) {
    if (type->isVoid())
        return;
    if (e == nullptr) {
        e = Location::regOf(8);
    }
    Signature::addReturn(type, e);
}

void CallingConvention::StdC::SparcSignature::addParameter(SharedType type, const QString &nam,
                                                           Exp *e, const QString &boundMax) {
    if (e == nullptr) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e, boundMax);
}

Exp *CallingConvention::StdC::SparcSignature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *e;
    if (n >= 6) {
        // SPARCs pass the seventh and subsequent parameters at m[%sp+92],
        // m[%esp+96], etc.
        e = Location::memOf(Binary::get(opPlus,
                                        Location::regOf(14), // %o6 == %sp
                                        new Const(92 + (n - 6) * 4)));
    } else
        e = Location::regOf(8 + n);
    return e;
}

Signature *CallingConvention::StdC::SparcSignature::promote(UserProc * /*p*/) {
    // no promotions from here up, obvious example would be name mangling
    return this;
}

Exp *CallingConvention::StdC::SparcSignature::getStackWildcard() {
    return Location::memOf(Binary::get(opPlus, Location::regOf(14), new Terminal(opWild)));
}

Exp *CallingConvention::StdC::SparcSignature::getProven(Exp *left) {
    if (left->isRegOfK()) {
        int r = ((Const *)((Location *)left)->getSubExp1())->getInt();
        switch (r) {
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

bool CallingConvention::StdC::SparcSignature::isPreserved(Exp *e) {
    if (e->isRegOfK()) {
        int r = ((Const *)((Location *)e)->getSubExp1())->getInt();
        switch (r) {
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

// Return a list of locations defined by library calls
void CallingConvention::StdC::SparcSignature::setLibraryDefines(StatementList *defs) {
    if (defs->size())
        return; // Do only once
    for (int r = 8; r <= 15; ++r)
        defs->append(new ImplicitAssign(Location::regOf(r))); // o0-o7 (r8-r15) modified
}

Exp *CallingConvention::StdC::SparcLibSignature::getProven(Exp *left) {
    if (left->isRegOfK()) {
        int r = ((Const *)((Location *)left)->getSubExp1())->getInt();
        switch (r) {
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

Signature::Signature(const QString &nam)
    : rettype(VoidType::get()), ellipsis(false), unknown(true), forced(false), preferedReturn(nullptr) {
    if (nam == nullptr)
        name = "<ANON>";
    else {
        name = nam;
        if (name == "__glutWarning") {
            qDebug() << name;
        }
    }
}

CustomSignature::CustomSignature(const QString &nam) : Signature(nam), sp(0) {}

void CustomSignature::setSP(int nsp) {
    sp = nsp;
    if (sp) {
        addReturn(Location::regOf(sp));
        // addImplicitParameter(PointerType::get(new IntegerType()), "sp",
        //                            Location::regOf(sp), nullptr);
    }
}

Signature *Signature::clone() {
    Signature *n = new Signature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->preferedName = preferedName;
    n->preferedReturn = preferedReturn ? preferedReturn->clone() : nullptr;
    n->preferedParams = preferedParams;
    n->unknown = unknown;
    n->sigFile = sigFile;
    return n;
}

Signature *CustomSignature::clone() {
    CustomSignature *n = new CustomSignature(name);
    cloneVec(params, n->params);
    // cloneVec(implicitParams, n->implicitParams);
    cloneVec(returns, n->returns);
    n->ellipsis = ellipsis;
    n->rettype = rettype->clone();
    n->sp = sp;
    n->forced = forced;
    n->preferedName = preferedName;
    n->preferedReturn = preferedReturn ? preferedReturn->clone() : nullptr;
    n->preferedParams = preferedParams;
    n->unknown = unknown;
    n->sigFile = sigFile;
    return n;
}

bool Signature::operator==(Signature &other) {
    // if (name != other.name) return false;        // MVE: should the name be significant? I'm thinking no
    if (params.size() != other.params.size())
        return false;
    // Only care about the first return location (at present)
    std::vector<Parameter *>::iterator it1, it2;
    for (it1 = params.begin(), it2 = other.params.begin(); it1 != params.end(); it1++, it2++)
        if (!(**it1 == **it2))
            return false;
    if (returns.size() != other.returns.size())
        return false;
    std::vector<Return *>::iterator rr1, rr2;
    for (rr1 = returns.begin(), rr2 = other.returns.begin(); rr1 != returns.end(); ++rr1, ++rr2)
        if (!(**rr1 == **rr2))
            return false;
    return true;
}

QString Signature::getName() { return name; }

void Signature::setName(const QString &nam) { name = nam; }

void Signature::addParameter(const char *nam /*= nullptr*/) { addParameter(VoidType::get(), nam); }

void Signature::addParameter(Exp *e, SharedType ty) { addParameter(ty, nullptr, e); }

void Signature::addParameter(SharedType type, const QString &nam /*= nullptr*/, Exp *e /*= nullptr*/,
                             const QString &boundMax /*= ""*/) {
    if (e == nullptr) {
        LOG_STREAM() << "No expression for parameter ";
        if (type == nullptr)
            LOG_STREAM() << "<notype> ";
        else
            LOG_STREAM() << type->getCtype() << " ";
        if (nam.isNull())
            LOG_STREAM() << "<noname>";
        else
            LOG_STREAM() << nam;
        LOG_STREAM() << "\n";
        assert(e); // Else get infinite mutual recursion with the below proc
    }

    QString s;
    QString new_name=nam;
    if (nam.isNull()) {
        size_t n = params.size() + 1;
        bool ok = false;
        while (!ok) {
            s = QString("param%1").arg(n);
            ok = true;
            for (auto &elem : params)
                if (s == elem->name()) {
                    ok = false;
                    break;
                }
            n++;
        }
        new_name = s;
    }
    Parameter *p = new Parameter(type, new_name, e, boundMax);
    addParameter(p);
    // addImplicitParametersFor(p);
}

void Signature::addParameter(Parameter *param) {
    SharedType ty = param->getType();
    QString nam = param->name();
    Exp *e = param->getExp();

    if (nam.isEmpty())
        nam = QString::null;

    if (ty == nullptr || e == nullptr || nam.isNull()) {
        addParameter(ty, nam, e, param->getBoundMax());
    } else
        params.push_back(param);
}

void Signature::removeParameter(Exp *e) {
    int i = findParam(e);
    if (i != -1)
        removeParameter(i);
}

void Signature::removeParameter(size_t i) {
    for (size_t j = i + 1; j < params.size(); j++)
        params[j - 1] = params[j];
    params.resize(params.size() - 1);
}

void Signature::setNumParams(size_t n) {
    if (n < params.size()) {
        // truncate
        params.erase(params.begin() + n, params.end());
    } else {
        for (size_t i = params.size(); i < n; i++)
            addParameter();
    }
}

const QString &Signature::getParamName(size_t n) {
    assert(n < params.size());
    return params[n]->name();
}

Exp *Signature::getParamExp(int n) {
    assert(n < (int)params.size());
    return params[n]->getExp();
}

SharedType Signature::getParamType(int n) {
    // assert(n < (int)params.size() || ellipsis);
    // With recursion, parameters not set yet. Hack for now:
    if (n >= (int)params.size())
        return nullptr;
    return params[n]->getType();
}

QString Signature::getParamBoundMax(int n) {
    if (n >= (int)params.size())
        return QString::null;
    QString s = params[n]->getBoundMax();
    if (s.isEmpty())
        return QString::null;
    return s;
}

void Signature::setParamType(int n, SharedType ty) { params[n]->setType(ty); }

void Signature::setParamType(const char *nam, SharedType ty) {
    int idx = findParam(nam);
    if (idx == -1) {
        LOG << "could not set type for unknown parameter " << nam << "\n";
        return;
    }
    params[idx]->setType(ty);
}

void Signature::setParamType(Exp *e, SharedType ty) {
    int idx = findParam(e);
    if (idx == -1) {
        LOG << "could not set type for unknown parameter expression " << e << "\n";
        return;
    }
    params[idx]->setType(ty);
}

void Signature::setParamName(int n, const char *name) { params[n]->name(name); }

void Signature::setParamExp(int n, Exp *e) { params[n]->setExp(e); }

// Return the index for the given expression, or -1 if not found
int Signature::findParam(Exp *e) {
    for (unsigned i = 0; i < getNumParams(); i++)
        if (*getParamExp(i) == *e)
            return i;
    return -1;
}

void Signature::renameParam(const QString &oldName, const char *newName) {
    for (unsigned i = 0; i < getNumParams(); i++)
        if (params[i]->name() ==oldName) {
            params[i]->name(newName);
            break;
        }
}

int Signature::findParam(const QString &nam) {
    for (unsigned i = 0; i < getNumParams(); i++)
        if (getParamName(i)==nam)
            return i;
    return -1;
}

int Signature::findReturn(Exp *e) {
    for (unsigned i = 0; i < getNumReturns(); i++)
        if (*returns[i]->exp == *e)
            return (int)i;
    return -1;
}

void Signature::addReturn(SharedType type, Exp *exp) {
    assert(exp);
    addReturn(new Return(type, exp));
    //    rettype = type->clone();
}

// Deprecated. Use the above version.
void Signature::addReturn(Exp *exp) {
    // addReturn(exp->getType() ? exp->getType() : new IntegerType(), exp);
    addReturn(std::make_shared<VoidType>(), exp);
}

void Signature::removeReturn(Exp *e) {
    int i = findReturn(e);
    if (i != -1) {
        for (unsigned j = i + 1; j < returns.size(); j++)
            returns[j - 1] = returns[j];
        returns.resize(returns.size() - 1);
    }
}

void Signature::setReturnType(size_t n, SharedType ty) {
    if (n < returns.size())
        returns[n]->type = ty;
}

Exp *Signature::getArgumentExp(int n) { return getParamExp(n); }
//! any signature can be promoted to a higher level signature, if available
Signature *Signature::promote(UserProc *p) {
    // FIXME: the whole promotion idea needs a redesign...
    if (CallingConvention::Win32Signature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::Win32Signature(*this);
        //        sig->analyse(p);
        delete this;
        return sig;
    }

    if (CallingConvention::StdC::PentiumSignature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::StdC::PentiumSignature(*this);
        //        sig->analyse(p);
        delete this;
        return sig;
    }

    if (CallingConvention::StdC::SparcSignature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::StdC::SparcSignature(*this);
        //        sig->analyse(p);
        delete this;
        return sig;
    }

    if (CallingConvention::StdC::PPCSignature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::StdC::PPCSignature(*this);
        //        sig->analyse(p);
        delete this;
        return sig;
    }

    if (CallingConvention::StdC::ST20Signature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::StdC::ST20Signature(*this);
        //        sig->analyse(p);
        delete this;
        return sig;
    }

    return this;
}

Signature *Signature::instantiate(platform plat, callconv cc, const QString &nam) {
    switch (plat) {
    case PLAT_PENTIUM:
        if (cc == CONV_PASCAL)
            // For now, assume the only pascal calling convention pentium signatures will be Windows
            return new CallingConvention::Win32Signature(nam);
        else if (cc == CONV_THISCALL)
            return new CallingConvention::Win32TcSignature(nam);
        else
            return new CallingConvention::StdC::PentiumSignature(nam);
    case PLAT_SPARC:
        if(cc == CONV_PASCAL)
            cc = CONV_C;
        assert(cc == CONV_C);
        return new CallingConvention::StdC::SparcSignature(nam);
    case PLAT_PPC:
        if(cc == CONV_PASCAL)
            cc = CONV_C;
        return new CallingConvention::StdC::PPCSignature(nam);
    case PLAT_ST20:
        if(cc == CONV_PASCAL)
            cc = CONV_C;
        return new CallingConvention::StdC::ST20Signature(nam);
    case PLAT_MIPS:
        if(cc == CONV_PASCAL)
            cc = CONV_C;
        return new CallingConvention::StdC::MIPSSignature(nam);
    // insert other conventions here
    default:
        qCritical() << "unknown signature: " << conventionName(cc) << " " << platformName(plat) << "\n";
        return nullptr;
    }
    return nullptr;
}

Signature::~Signature() {
    qDeleteAll(params);
}

void Signature::print(QTextStream &out, bool /*html*/) const {
    if (isForced())
        out << "*forced* ";
    if (returns.size() > 0) {
        out << "{ ";
        unsigned n = 0;
        for (const Return *rr : returns) {
            out << rr->type->getCtype() << " " << rr->exp;
            if (n != returns.size() - 1)
                out << ",";
            out << " ";
            n++;
        }
        out << "} ";
    } else
        out << "void ";
    out << name << "(";
    unsigned int i;
    for (i = 0; i < params.size(); i++) {
        out << params[i]->getType()->getCtype() << " " << params[i]->name() << " " << params[i]->getExp();
        if (i != params.size() - 1)
            out << ", ";
    }
    out << ")\n";
}

char *Signature::prints() {
    QString tgt;
    QTextStream ost(&tgt);
    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}

void Signature::printToLog() {
    QString tgt;
    QTextStream os(&tgt);
    print(os);
    LOG << tgt;
}

bool Signature::usesNewParam(UserProc * /*p*/, Instruction *stmt, bool checkreach, int &n) {
    QTextStream q_cerr(stderr);
    n = getNumParams() - 1;
    if (VERBOSE) {
        q_cerr << "searching ";
        stmt->printAsUse(q_cerr);
        q_cerr << '\n';
    }
    InstructionSet reachin;

    // stmt->getReachIn(reachin, 2);
    for (int i = getNumParams(); i < 10; i++)
        if (stmt->usesExp(*getParamExp(i))) {
            bool ok = true;
            if (checkreach) {
                bool hasDef = false;
                InstructionSet::iterator it1;
                for (it1 = reachin.begin(); it1 != reachin.end(); it1++) {
                    Assignment *as = (Assignment *)*it1;
                    if (as->isAssignment() && *as->getLeft() == *getParamExp(i)) {
                        hasDef = true;
                        break;
                    }
                }
                if (hasDef)
                    ok = false;
            }
            if (ok) {
                n = i;
            }
        }
    return n > ((int)getNumParams() - 1);
}

// Special for Mike: find the location where the first outgoing (actual) parameter is conventionally held
Exp *Signature::getFirstArgLoc(Prog *prog) {
    MACHINE mach = prog->getMachine();
    switch (mach) {
    case MACHINE_SPARC: {
        CallingConvention::StdC::SparcSignature sig("");
        return sig.getArgumentExp(0);
    }
    case MACHINE_PENTIUM: {
        // CallingConvention::StdC::PentiumSignature sig("");
        // Exp* e = sig.getArgumentExp(0);
        // For now, need to work around how the above appears to be the wrong thing!
        Exp *e = Location::memOf(Location::regOf(28));
        return e;
    }
    case MACHINE_ST20: {
        CallingConvention::StdC::ST20Signature sig("");
        return sig.getArgumentExp(0);
        // return Location::regOf(0);
    }
    default:
        LOG_STREAM() << "Signature::getFirstArgLoc: machine not handled\n";
        assert(0);
    }
    return nullptr;
}

// A bit of a cludge. Problem is that we can't call the polymorphic getReturnExp() until signature promotion has
// happened. For the switch logic, that happens way too late. So for now, we have this cludge.
// This is very very hacky! (trent)
/*static*/ Exp *Signature::getReturnExp2(LoaderInterface *pBF) {
    switch (pBF->getMachine()) {
    case MACHINE_SPARC:
        return Location::regOf(8);
    case MACHINE_PENTIUM:
        return Location::regOf(24);
    case MACHINE_ST20:
        return Location::regOf(0);
    default:
        LOG_STREAM() << "getReturnExp2: machine not handled\n";
    }
    return nullptr;
}

// Not very satisfying to do things this way. Problem is that the polymorphic CallingConvention objects are set up
// very late in the decompilation. Get the set of registers that are not saved in library functions (or any
// procedures that follow the calling convention)
void Signature::setABIdefines(Prog *prog, StatementList *defs) {
    if (defs->size())
        return; // Do only once
    MACHINE mach = prog->getMachine();
    switch (mach) {
    case MACHINE_PENTIUM: {
        defs->append(new ImplicitAssign(Location::regOf(24))); // eax
        defs->append(new ImplicitAssign(Location::regOf(25))); // ecx
        defs->append(new ImplicitAssign(Location::regOf(26))); // edx
        break;
    }
    case MACHINE_SPARC: {
        for (int r = 8; r <= 13; ++r)
            defs->append(new ImplicitAssign(Location::regOf(r))); // %o0-o5
        defs->append(new ImplicitAssign(Location::regOf(1)));     // %g1
        break;
    }
    case MACHINE_PPC: {
        for (int r = 3; r <= 12; ++r)
            defs->append(new ImplicitAssign(Location::regOf(r))); // r3-r12
        break;
    }
    case MACHINE_ST20: {
        defs->append(new ImplicitAssign(Location::regOf(0))); // A
        defs->append(new ImplicitAssign(Location::regOf(1))); // B
        defs->append(new ImplicitAssign(Location::regOf(2))); // C
        break;
    }
    default:
        break;
    }
}

// Get the expected argument location, based solely on the machine of the input program
Exp *Signature::getEarlyParamExp(int n, Prog *prog) {
    MACHINE mach = prog->getMachine();
    switch (mach) {
    case MACHINE_SPARC: {
        CallingConvention::StdC::SparcSignature temp("");
        return temp.getParamExp(n);
    }
    case MACHINE_PENTIUM: {
        // Would we ever need Win32?
        CallingConvention::StdC::PentiumSignature temp("");
        return temp.getParamExp(n);
    }
    case MACHINE_ST20: {
        CallingConvention::StdC::ST20Signature temp("");
        return temp.getParamExp(n);
    }
    default:
        break;
    }
    assert(0); // Machine not handled
    return nullptr;
}

StatementList &Signature::getStdRetStmt(Prog *prog) {
    // pc := m[r[28]]
    static Assign pent1ret(new Terminal(opPC), Location::memOf(Location::regOf(28)));
    // r[28] := r[28] + 4
    static Assign pent2ret(Location::regOf(28), Binary::get(opPlus, Location::regOf(28), new Const(4)));
    static Assign st20_1ret(new Terminal(opPC), Location::memOf(Location::regOf(3)));
    static Assign st20_2ret(Location::regOf(3), Binary::get(opPlus, Location::regOf(3), new Const(16)));
    MACHINE mach = prog->getMachine();
    switch (mach) {
    case MACHINE_SPARC:
        break; // No adjustment to stack pointer required
    case MACHINE_PENTIUM: {
        StatementList *sl = new StatementList;
        sl->append((Instruction *)&pent1ret);
        sl->append((Instruction *)&pent2ret);
        return *sl;
    }
    case MACHINE_ST20: {
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

int Signature::getStackRegister() noexcept(false) {
    if (VERBOSE)
        LOG << "thowing StackRegisterNotDefinedException\n";
    throw StackRegisterNotDefinedException();
}

// Needed before the signature is promoted
int Signature::getStackRegister(Prog *prog) noexcept(false) {
    MACHINE mach = prog->getMachine();
    switch (mach) {
    case MACHINE_SPARC:
        return 14;
    case MACHINE_PENTIUM:
        return 28;
    case MACHINE_PPC:
        return 1;
    case MACHINE_ST20:
        return 3;
    default:
        throw StackRegisterNotDefinedException();
    }
}
/**
Does expression e represent a local stack-based variable?
Result can be ABI specific, e.g. sparc has locals in the parent's stack frame, at POSITIVE offsets from the
stack pointer register
Also, I believe that the PA/RISC stack grows away from 0
*/
bool Signature::isStackLocal(Prog *prog, Exp *e) {
    // e must be m[...]
    if (e->isSubscript())
        return isStackLocal(prog, e->getSubExp1());
    if (!e->isMemOf())
        return false;
    Exp *addr = ((Location *)e)->getSubExp1();
    return isAddrOfStackLocal(prog, addr);
}

bool Signature::isAddrOfStackLocal(Prog *prog, Exp *e) {
    OPER op = e->getOper();
    if (op == opAddrOf)
        return isStackLocal(prog, e->getSubExp1());
    // e must be sp -/+ K or just sp
    static Exp *sp = Location::regOf(getStackRegister(prog));
    if (op != opMinus && op != opPlus) {
        // Matches if e is sp or sp{0} or sp{-}
        return (*e == *sp ||
                (e->isSubscript() && ((RefExp *)e)->isImplicitDef() && *((RefExp *)e)->getSubExp1() == *sp));
    }
    if (op == opMinus && !isLocalOffsetNegative())
        return false;
    if (op == opPlus && !isLocalOffsetPositive())
        return false;
    Exp *sub1 = ((Binary *)e)->getSubExp1();
    Exp *sub2 = ((Binary *)e)->getSubExp2();
    // e must be <sub1> +- K
    if (!sub2->isIntConst())
        return false;
    // first operand must be sp or sp{0} or sp{-}
    if (sub1->isSubscript()) {
        if (!((RefExp *)sub1)->isImplicitDef())
            return false;
        sub1 = ((RefExp *)sub1)->getSubExp1();
    }
    return *sub1 == *sp;
}

// An override for the SPARC: [sp+0] .. [sp+88] are local variables (effectively), but [sp + >=92] are memory parameters
bool CallingConvention::StdC::SparcSignature::isAddrOfStackLocal(Prog *prog, Exp *e) {
    OPER op = e->getOper();
    if (op == opAddrOf)
        return isStackLocal(prog, e->getSubExp1());
    // e must be sp -/+ K or just sp
    static Exp *sp = Location::regOf(14);
    if (op != opMinus && op != opPlus) {
        // Matches if e is sp or sp{0} or sp{-}
        return (*e == *sp ||
                (e->isSubscript() && ((RefExp *)e)->isImplicitDef() && *((RefExp *)e)->getSubExp1() == *sp));
    }
    Exp *sub1 = ((Binary *)e)->getSubExp1();
    Exp *sub2 = ((Binary *)e)->getSubExp2();
    // e must be <sub1> +- K
    if (!sub2->isIntConst())
        return false;
    // first operand must be sp or sp{0} or sp{-}
    if (sub1->isSubscript()) {
        if (!((RefExp *)sub1)->isImplicitDef())
            return false;
        sub1 = ((RefExp *)sub1)->getSubExp1();
    }
    if (!(*sub1 == *sp))
        return false;
    // SPARC specific test: K must be < 92; else it is a parameter
    int K = ((Const *)sub2)->getInt();
    return K < 92;
}

bool Parameter::operator==(Parameter &other) {
    if (!(*type == *other.type))
        return false;
    // Do we really care about a parameter's name?
    if (!(m_name == other.m_name))
        return false;
    if (!(*exp == *other.exp))
        return false;
    return true;
}

// bool CallingConvention::StdC::HppaSignature::isLocalOffsetPositive() {
//      return true;
//}

bool Signature::isOpCompatStackLocal(OPER op) {
    if (op == opMinus)
        return isLocalOffsetNegative();
    if (op == opPlus)
        return isLocalOffsetPositive();
    return false;
}

bool Signature::returnCompare(Assignment &a, Assignment &b) {
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}

bool Signature::argumentCompare(Assignment &a, Assignment &b) {
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}

bool CallingConvention::StdC::PentiumSignature::returnCompare(Assignment &a, Assignment &b) {
    Exp *la = a.getLeft();
    Exp *lb = b.getLeft();
    // Eax is the preferred return location
    if (la->isRegN(24))
        return true; // r24 is less than anything
    if (lb->isRegN(24))
        return false; // Nothing is less than r24

    // Next best is r30 (floating point %st)
    if (la->isRegN(30))
        return true; // r30 is less than anything that's left
    if (lb->isRegN(30))
        return false; // Nothing left is less than r30

    // Else don't care about the order
    return *la < *lb;
}

static Unary spPlus64(opMemOf, Binary::get(opPlus, Location::regOf(14), new Const(64)));
bool CallingConvention::StdC::SparcSignature::returnCompare(Assignment &a, Assignment &b) {
    Exp *la = a.getLeft();
    Exp *lb = b.getLeft();
    // %o0 (r8) is the preferred return location
    if (la->isRegN(8))
        return true; // r24 is less than anything
    if (lb->isRegN(8))
        return false; // Nothing is less than r24

    // Next best is %f0 (r32)
    if (la->isRegN(32))
        return true; // r32 is less than anything that's left
    if (lb->isRegN(32))
        return false; // Nothing left is less than r32

    // Next best is %f0-1 (r64)
    if (la->isRegN(64))
        return true; // r64 is less than anything that's left
    if (lb->isRegN(64))
        return false; // Nothing left is less than r64

    // Next best is m[esp{-}+64]
    if (*la == spPlus64)
        return true; // m[esp{-}+64] is less than anything that's left
    if (*lb == spPlus64)
        return false; // Nothing left is less than m[esp{-}+64]

    // Else don't care about the order
    return *la < *lb;
}

// From m[sp +- K] return K (or -K for subtract). sp could be subscripted with {-}
// Helper function for the below
int stackOffset(Exp *e, int sp) {
    int ret = 0;
    if (e->isMemOf()) {
        Exp *sub = ((Location *)e)->getSubExp1();
        OPER op = sub->getOper();
        if (op == opPlus || op == opMinus) {
            Exp *op1 = ((Binary *)sub)->getSubExp1();
            if (op1->isSubscript())
                op1 = ((RefExp *)op1)->getSubExp1();
            if (op1->isRegN(sp)) {
                Exp *op2 = ((Binary *)sub)->getSubExp2();
                if (op2->isIntConst())
                    ret = ((Const *)op2)->getInt();
                if (op == opMinus)
                    ret = -ret;
            }
        }
    }
    return ret;
}

bool CallingConvention::StdC::PentiumSignature::argumentCompare(Assignment &a, Assignment &b) {
    Exp *la = a.getLeft();
    Exp *lb = b.getLeft();
    int ma = stackOffset(la, 28);
    int mb = stackOffset(lb, 28);

    if (ma && mb)
        return ma < mb;
    if (ma && !mb)
        return true; // m[sp-K] is less than anything else
    if (mb && !ma)
        return false; // Nothing else is less than m[sp-K]

    // Else don't care about the order
    return *la < *lb;
}

bool CallingConvention::StdC::SparcSignature::argumentCompare(Assignment &a, Assignment &b) {
    Exp *la = a.getLeft();
    Exp *lb = b.getLeft();
    // %o0-$o5 (r8-r13) are the preferred argument locations
    int ra = 0, rb = 0;
    if (la->isRegOf()) {
        int r = ((Const *)((Location *)la)->getSubExp1())->getInt();
        if (r >= 8 && r <= 13)
            ra = r;
    }
    if (lb->isRegOf()) {
        int r = ((Const *)((Location *)lb)->getSubExp1())->getInt();
        if (r >= 8 && r <= 13)
            rb = r;
    }
    if (ra && rb)
        return ra < rb; // Both r8-r13: compare within this set
    if (ra && rb == 0)
        return true; // r8-r13 less than anything else
    if (rb && ra == 0)
        return false; // Nothing else is less than r8-r13

    int ma = stackOffset(la, 30);
    int mb = stackOffset(lb, 30);

    if (ma && mb)
        return ma < mb; // Both m[sp + K]: order by memory offset
    if (ma && !mb)
        return true; // m[sp+K] less than anything left
    if (mb && !ma)
        return false; // nothing left is less than m[sp+K]

    return *la < *lb; // Else order arbitrarily
}

// Class Return methods
Return *Return::clone() { return new Return(type->clone(), exp->clone()); }

bool Return::operator==(Return &other) {
    if (!(*type == *other.type))
        return false;
    if (!(*exp == *other.exp))
        return false;
    return true;
}

SharedType Signature::getTypeFor(Exp *e) {
    size_t n = returns.size();
    for (size_t i = 0; i < n; ++i) {
        if (*returns[i]->exp == *e)
            return returns[i]->type;
    }
    return nullptr;
}
