/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       signature.cpp
 * OVERVIEW:   Implementation of the classes that describe a procedure signature
 *============================================================================*/

/*
 * $Revision$
 * 
 * 15 Jul 02 - Trent: Created.
 * 18 Jul 02 - Mike: Changed addParameter's last param to deflt to "", not NULL
 * 02 Jan 03 - Mike: Fixed SPARC getParamExp and getArgExp
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <string>
#include <sstream>
#include "type.h"
#include "signature.h"
#include "exp.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "signature.h"
#include "util.h"
#include "cfg.h"
#include "proc.h"
#include "boomerang.h"

char* Signature::platformName(platform plat) {
    switch (plat) {
        case PLAT_PENTIUM:  return "pentium";
        case PLAT_SPARC:    return "sparc";
        case PLAT_M68K:     return "m68k";
        case PLAT_PARISC:   return "parisc";
        case PLAT_PPC:      return "ppc";
        case PLAT_MIPS:     return "mips";
        default:            return "???";
    }
}

char* Signature::conventionName(callconv cc) {
    switch (cc) {
        case CONV_C:        return "stdc";
        case CONV_PASCAL:   return "pascal";
        default:            return "??";
    }
}

namespace CallingConvention {

    class Win32Signature : public Signature {
    public:
        Win32Signature(const char *nam);
        Win32Signature(Signature &old);
        virtual ~Win32Signature() { }
        virtual Signature *clone();
        virtual bool operator==(Signature& other);
        static bool qualified(UserProc *p, Signature &candidate);

        virtual void addReturn(Type *type, Exp *e = NULL);
        virtual void addParameter(Type *type, const char *nam = NULL, 
                                  Exp *e = NULL);
        virtual Exp *getArgumentExp(int n);

        virtual Signature *promote(UserProc *p);
        virtual void getInternalStatements(StatementList &stmts);
        virtual Exp *getStackWildcard();
        virtual int  getStackRegister() {return 28; }
        virtual Exp *getProven(Exp *left);

        virtual bool isPromoted() { return true; }
    };

    namespace StdC {
        class PentiumSignature : public Signature {
        public:
            PentiumSignature(const char *nam);
            PentiumSignature(Signature &old);
            virtual ~PentiumSignature() { }
            virtual Signature *clone(); 
            virtual bool operator==(Signature& other);
            static bool qualified(UserProc *p, Signature &candidate);

            virtual void addReturn(Type *type, Exp *e = NULL);
            virtual void addParameter(Type *type, const char *nam = NULL, 
                                      Exp *e = NULL);
            virtual Exp *getArgumentExp(int n);

            virtual Signature *promote(UserProc *p);
            virtual void getInternalStatements(StatementList &stmts);
            virtual Exp *getStackWildcard();
            virtual int  getStackRegister() {return 28; }
            virtual Exp *getProven(Exp *left);
            virtual bool isPromoted() { return true; }
        };      

        class SparcSignature : public Signature {
        public:
            SparcSignature(const char *nam);
            SparcSignature(Signature &old);
            virtual ~SparcSignature() { }
            virtual Signature *clone();
            virtual bool operator==(Signature& other);
            static bool qualified(UserProc *p, Signature &candidate);

            virtual void addReturn(Type *type, Exp *e = NULL);
            virtual void addParameter(Type *type, const char *nam = NULL, 
                                      Exp *e = NULL);
            virtual Exp *getArgumentExp(int n);

            virtual Signature *promote(UserProc *p);
            virtual Exp *getStackWildcard();
            virtual int  getStackRegister() {return 14; }
            virtual bool isPromoted() { return true; }
        };
    };
};

CallingConvention::Win32Signature::Win32Signature(const char *nam) : Signature(nam)
{
    Signature::addReturn(Location::regOf(28));
    Signature::addImplicitParameter(new PointerType(new IntegerType()), "esp",
                                    Location::regOf(28), NULL);
}

CallingConvention::Win32Signature::Win32Signature(Signature &old) : Signature(old)
{
}

Signature *CallingConvention::Win32Signature::clone()
{
    Win32Signature *n = new Win32Signature(name.c_str());
    n->params = params;
    n->implicitParams = implicitParams;
    n->returns = returns;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

bool CallingConvention::Win32Signature::operator==(Signature& other)
{
    return Signature::operator==(other);
}

bool CallingConvention::Win32Signature::qualified(UserProc *p,
  Signature &candidate) {
    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_PENTIUM || !p->getProg()->isWin32()) return false;

    if (VERBOSE) {
        std::cerr << "consider promotion to stdc win32 signature for " <<
          p->getName() << std::endl;
    }

    bool gotcorrectret1 = false;
    bool gotcorrectret2 = false;
    StatementList internal;
    // FIXME: Below is never executed now
    //p->getInternalStatements(internal);
    StatementList::iterator it;
    for (it = internal.begin(); it != internal.end(); it++) {
        Assign *e = dynamic_cast<Assign*>(*it);
        if (e == NULL) continue;
        if (VERBOSE) {
            std::cerr << "internal: ";
            e->print(std::cerr);
            std::cerr << std::endl;
        }
        if (e->getLeft()->getOper() == opPC) {
            if (e->getRight()->isMemOf() && 
              e->getRight()->getSubExp1()->isRegOf() &&
              e->getRight()->getSubExp1()->getSubExp1()->isIntConst() &&
              ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt()
              == 28) {
                if (VERBOSE)
                    std::cerr << "got pc = m[r[28]]" << std::endl;
                gotcorrectret1 = true;
            }
        } else if (e->getLeft()->isRegOf() && 
                   e->getLeft()->getSubExp1()->isIntConst() &&
                   ((Const*)e->getLeft()->getSubExp1())->getInt() == 28) {
            if (e->getRight()->getOper() == opPlus &&
              e->getRight()->getSubExp1()->isRegOf() &&
              e->getRight()->getSubExp1()->getSubExp1()->isIntConst() &&
              ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt()
              == 28 &&
              e->getRight()->getSubExp2()->isIntConst() &&
              ((Const*)e->getRight()->getSubExp2())->getInt() == 4) {
                if (VERBOSE)
                    std::cerr << "got r[28] = r[28] + 4" << std::endl;
                gotcorrectret2 = true;
            }
        }
    }
    return gotcorrectret1 && gotcorrectret2;
}

void CallingConvention::Win32Signature::addReturn(Type *type, Exp *e)
{
    if (type->isVoid())
        return;
    if (e == NULL) {
        if (type->isFloat())
            e = Location::regOf(32);
        else 
            e = Location::regOf(24);
    }
    Signature::addReturn(type, e);
}
    
void CallingConvention::Win32Signature::addParameter(Type *type, 
                             const char *nam /*= NULL*/, Exp *e /*= NULL*/)
{
    if (e == NULL) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e);
}

Exp *CallingConvention::Win32Signature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *esp = Location::regOf(28);
    if (params.size() != 0 && *params[0]->getExp() == *esp)
        n--;
    Exp *e = Location::memOf(new Binary(opPlus, esp, new Const((n+1) * 4)));
    return e;
}

Signature *CallingConvention::Win32Signature::promote(UserProc *p)
{
    // no promotions from win32 signature up, yet.
    // a possible thing to investigate would be COM objects
    return this;
}

Exp *CallingConvention::Win32Signature::getStackWildcard() {
    // Note: m[esp + -8] is simnplified to m[esp - 8] now
    return Location::memOf(new Binary(opMinus, Location::regOf(28), 
                                               new Terminal(opWild)));
}

Exp *CallingConvention::Win32Signature::getProven(Exp *left)
{
    int nparams = params.size();
    if (nparams > 0 && *params[0]->getExp() == *Location::regOf(28)) {
        nparams--;
    }
    if (left->getOper() == opRegOf && 
        left->getSubExp1()->getOper() == opIntConst) {
        switch (((Const*)left->getSubExp1())->getInt()) {
            case 28:
                return new Binary(opPlus, Location::regOf(28), 
                                          new Const(4 + nparams*4));
            case 26:
                return Location::regOf(29);
            case 27:
                return Location::regOf(29);
            case 29:
                return Location::regOf(29);
            case 30:
                return Location::regOf(30);
            case 31:
                return Location::regOf(31);
            // there are other things that must be preserved here, look at calling convention
        }
    }
    return NULL;
}

void CallingConvention::Win32Signature::getInternalStatements(StatementList &stmts)
{
    static Assign *fixpc = new Assign(new Terminal(opPC),
            Location::memOf(Location::regOf(28)));
    static Assign *fixesp = new Assign(Location::regOf(28),
            new Binary(opPlus, Location::regOf(28),
                new Const(4 + params.size()*4)));
    stmts.append((Assign*)fixpc->clone());
    stmts.append((Assign*)fixesp->clone());
}

CallingConvention::StdC::PentiumSignature::PentiumSignature(const char *nam) : Signature(nam)
{
    Signature::addReturn(Location::regOf(28));
    Signature::addImplicitParameter(new PointerType(new IntegerType()), "esp",
                                    Location::regOf(28), NULL);
}

CallingConvention::StdC::PentiumSignature::PentiumSignature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::StdC::PentiumSignature::clone()
{
    PentiumSignature *n = new PentiumSignature(name.c_str());
    n->params = params;
    n->implicitParams = implicitParams;
    n->returns = returns;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

bool CallingConvention::StdC::PentiumSignature::operator==(Signature& other)
{
    return Signature::operator==(other);
}


// FIXME: This needs changing. Would like to check that pc=pc and sp=sp
// (or maybe sp=sp+4) for qualifying procs. Need work to get there
bool CallingConvention::StdC::PentiumSignature::qualified(UserProc *p,
  Signature &candidate) {
    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_PENTIUM) return false;

    if (VERBOSE)
        std::cerr << "consider promotion to stdc pentium signature for " <<
          p->getName() << std::endl;

#if 1
    return true;        // For now, always pass
#else
    bool gotcorrectret1 = false;
    bool gotcorrectret2 = false;
    StatementList internal;
    //p->getInternalStatements(internal);
    internal.append(*p->getCFG()->getReachExit());
    StmtListIter it;
    for (Statement* s = internal.getFirst(it); s; s = internal.getNext(it)) {
        Assign *e = dynamic_cast<Assign*>(s);
        if (e == NULL) continue;
        if (e->getLeft()->getOper() == opPC) {
            if (e->getRight()->isMemOf() && 
              e->getRight()->getSubExp1()->isRegOf() &&
              e->getRight()->getSubExp1()->getSubExp1()->isIntConst() &&
              ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt()
              == 28) {
                if (VERBOSE)
                    std::cerr << "got pc = m[r[28]]" << std::endl;
                gotcorrectret1 = true;
            }
        } else if (e->getLeft()->isRegOf() && 
          e->getLeft()->getSubExp1()->isIntConst() &&
          ((Const*)e->getLeft()->getSubExp1())->getInt() == 28) {
            if (e->getRight()->getOper() == opPlus &&
              e->getRight()->getSubExp1()->isRegOf() &&
              e->getRight()->getSubExp1()->getSubExp1()->isIntConst() &&
              ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt()
              == 28 &&
              e->getRight()->getSubExp2()->isIntConst() &&
              ((Const*)e->getRight()->getSubExp2())->getInt() == 4) {
                if (VERBOSE)
                    std::cerr << "got r[28] = r[28] + 4" << std::endl;
                gotcorrectret2 = true;
            }
        }
    }
    return gotcorrectret1 && gotcorrectret2;
#endif
}

void CallingConvention::StdC::PentiumSignature::addReturn(Type *type, Exp *e)
{
    if (type->isVoid())
        return;
    if (e == NULL) {
        if (type->isFloat())
            e = Location::regOf(32);
        else 
            e = Location::regOf(24);
    }
    Signature::addReturn(type, e);
}

void CallingConvention::StdC::PentiumSignature::addParameter(Type *type, 
                             const char *nam /*= NULL*/, Exp *e /*= NULL*/)
{
    if (e == NULL) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e);
}

Exp *CallingConvention::StdC::PentiumSignature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *esp = Location::regOf(28);
    if (params.size() != 0 && *params[0]->getExp() == *esp)
        n--;
    Exp *e = Location::memOf(new Binary(opPlus, esp, new Const((n+1) * 4)));
    return e;
}

Signature *CallingConvention::StdC::PentiumSignature::promote(UserProc *p)
{
    // No promotions from here up, obvious idea would be c++ name mangling  
    return this;
}

Exp *CallingConvention::StdC::PentiumSignature::getStackWildcard() {
    // Note: m[esp + -8] is simplified to m[esp - 8] now
    return Location::memOf(new Binary(opMinus, Location::regOf(28),
                                                   new Terminal(opWild)));
}

Exp *CallingConvention::StdC::PentiumSignature::getProven(Exp *left)
{
    if (left->getOper() == opRegOf && 
        left->getSubExp1()->getOper() == opIntConst) {
        switch (((Const*)left->getSubExp1())->getInt()) {
            case 28:
                return new Binary(opPlus, Location::regOf(28), new Const(4));
            case 29:
                return Location::regOf(29);
            // there are other things that must be preserved here, look at calling convention
        }
    }
    return NULL;
}

void CallingConvention::StdC::PentiumSignature::getInternalStatements(
  StatementList &stmts) {
    // pc := m[r28]
    static Assign *fixpc = new Assign(new Terminal(opPC),
            Location::memOf(Location::regOf(28)));
    // r28 := r28 + 4;
    static Assign *fixesp = new Assign(Location::regOf(28),
            new Binary(opPlus, Location::regOf(28),
                new Const(4)));
    stmts.append((Assign*)fixpc->clone());
    stmts.append((Assign*)fixesp->clone());
}

CallingConvention::StdC::SparcSignature::SparcSignature(const char *nam) :
  Signature(nam) {
}

CallingConvention::StdC::SparcSignature::SparcSignature(Signature &old) :
  Signature(old) {
}

Signature *CallingConvention::StdC::SparcSignature::clone() {
    SparcSignature *n = new SparcSignature(name.c_str());
    n->params = params;
    n->implicitParams = implicitParams;
    n->returns = returns;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

bool CallingConvention::StdC::SparcSignature::operator==(Signature&
  other) {
    return Signature::operator==(other);
}

bool CallingConvention::StdC::SparcSignature::qualified(UserProc *p,
  Signature &candidate) {
    platform plat = p->getProg()->getFrontEndId();
    if (plat != PLAT_SPARC) return false;

    // is there other constraints?
    
    return true;
}

void CallingConvention::StdC::SparcSignature::addReturn(Type *type, Exp *e)
{
    if (type->isVoid())
        return;
    if (e == NULL) {
        e = Location::regOf(8);
    }
    Signature::addReturn(type, e);
}

void CallingConvention::StdC::SparcSignature::addParameter(Type *type, 
                             const char *nam /*= NULL*/, Exp *e /*= NULL*/)
{
    if (e == NULL) {
        e = getArgumentExp(params.size());
    }
    Signature::addParameter(type, nam, e);
}

Exp *CallingConvention::StdC::SparcSignature::getArgumentExp(int n) {
    if (n < (int)params.size())
        return Signature::getArgumentExp(n);
    Exp *e;
    if (n >= 6) {
        // SPARCs pass the seventh and subsequent parameters at m[%sp+92],
        // m[%esp+96], etc.
        e = Location::memOf(new Binary(opPlus,
                Location::regOf(14), // %o6 == %sp
                new Const(92 + (n-6)*4)));
    } else
        e = Location::regOf((int)(8 + n));
    return e;
}
 
Signature *CallingConvention::StdC::SparcSignature::promote(UserProc *p)
{
    // no promotions from here up, obvious example would be name mangling
    return this;
}

Exp *CallingConvention::StdC::SparcSignature::getStackWildcard()
{
    return Location::memOf(new Binary(opPlus, Location::regOf(14),
               new Terminal(opWild)));
}

Signature::Signature(const char *nam) : rettype(new VoidType()), ellipsis(false)
{
    if (nam == NULL) 
        name = "<ANON>";
    else
        name = nam;
}

CustomSignature::CustomSignature(const char *nam) : Signature(nam), sp(0)
{
}

void CustomSignature::setSP(int nsp)
{
    sp = nsp;
    if (sp) {
        addReturn(Location::regOf(sp));
        addImplicitParameter(new PointerType(new IntegerType()), "sp",
                                    Location::regOf(sp), NULL);
    }
}

Signature *Signature::clone()
{
    Signature *n = new Signature(name.c_str());
    n->params = params;
    n->implicitParams = implicitParams;
    n->returns = returns;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

Signature *CustomSignature::clone()
{
    CustomSignature *n = new CustomSignature(name.c_str());
    n->params = params;
    n->implicitParams = implicitParams;
    n->returns = returns;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    n->sp = sp;
    return n;
}


bool Signature::operator==(Signature& other)
{
    if (name != other.name) return false;
    if (params.size() != other.params.size()) return false;
    // Only care about the first return location (at present)
    if ((returns.size() != 0) != (other.returns.size() != 0)) return false;
    std::vector<Parameter*>::iterator it1, it2;
    for (it1 = params.begin(), it2 = other.params.begin();
      it1 != params.end();
      it1++, it2++)
        if (!(**it1 == **it2)) return false; 
    if (returns.size())
        // Compare the first return type only
        if (!(*returns[0] == *other.returns[0])) return false; 
    return true;
}

const char *Signature::getName()
{
    return name.c_str();
}

void Signature::setName(const char *nam)
{
    name = nam;
}

void Signature::addParameter(const char *nam /*= NULL*/)
{
    addParameter(new IntegerType(), nam);
}

void Signature::addParameter(Exp *e)
{
    addParameter(new IntegerType(), NULL, e);
}

void Signature::addParameter(Type *type, const char *nam /*= NULL*/, 
                             Exp *e /*= NULL*/)
{
    if (e == NULL) {
        std::cerr << "No expression for parameter ";
        if (type == NULL)
            std::cerr << "<notype> ";
        else
            std::cerr << type->getCtype() << " ";
        if (nam == NULL)
            std::cerr << "<noname>";
        else
            std::cerr << nam;
        std::cerr << "\n";
        assert(e);  // Else get infinite mutual recursion with the below proc
    }

    std::string s;
    if (nam == NULL) {
        int n = params.size()+1;
        bool ok = false;
        while (!ok) {
            std::stringstream os;
            os << "param" << n << std::ends;
            s = os.str();
            ok = true;
            for (unsigned i = 0; i < params.size(); i++)
                if (!strcmp(s.c_str(), params[i]->getName()))
                    ok = false;
            n++;
        }
        nam = s.c_str();
    }
    Parameter *p = new Parameter(type, nam, e); 
    addParameter(p);
    addImplicitParametersFor(p);
}

void Signature::addParameter(Parameter *param)
{
    Type *ty = param->getType();
    const char *nam = param->getName();
    Exp *e = param->getExp();

    if (strlen(nam) == 0)
        nam = NULL;

    if (ty == NULL || e == NULL || nam == NULL) {
        addParameter(ty, nam, e);
    } else
        params.push_back(param);
}

void Signature::removeParameter(Exp *e)
{
    int i = findParam(e);
    if (i != -1)
        removeParameter(i);
}

void Signature::removeParameter(int i)
{
    for (unsigned j = i+1; j < params.size(); j++)
        params[j-1] = params[j];
    params.resize(params.size()-1);
}

void Signature::setNumParams(int n)
{
    if (n < (int)params.size()) {
        // truncate
        params.erase(params.begin() + n, params.end());
    } else {
        for (int i = params.size(); i < n; i++)
            addParameter();     
    }
}

int Signature::getNumParams() {
    return params.size();
}

const char *Signature::getParamName(int n) {
    assert(n < (int)params.size());
    return params[n]->getName();
}

Exp *Signature::getParamExp(int n) {
    assert(n < (int)params.size());
    return params[n]->getExp();
}

Type *Signature::getParamType(int n) {
    static IntegerType def;
    //assert(n < (int)params.size() || ellipsis);
// With recursion, parameters not set yet. Hack for now:
    if (n >= (int)params.size()) return &def;
    return params[n]->getType();
}

void Signature::setParamType(int n, Type *ty) {
    params[n]->setType(ty);
}

int Signature::findParam(Exp *e) {
    for (int i = 0; i < getNumParams(); i++)
        if (*getParamExp(i) == *e)
            return i;
    return -1;
}

int Signature::findParam(const char *nam) {
    for (int i = 0; i < getNumParams(); i++)
        if (!strcmp(getParamName(i), nam))
            return i;
    return -1;
}

int Signature::getNumImplicitParams() {
    return implicitParams.size();
}

const char *Signature::getImplicitParamName(int n) {
    assert(n < (int)implicitParams.size());
    return implicitParams[n]->getName();
}

Exp *Signature::getImplicitParamExp(int n) {
    assert(n < (int)implicitParams.size());
    return implicitParams[n]->getExp();
}

Type *Signature::getImplicitParamType(int n) {
    static IntegerType def;
    //assert(n < (int)params.size() || ellipsis);
// With recursion, parameters not set yet. Hack for now:  (do we still need this?  - trent)
    if (n >= (int)implicitParams.size()) return &def;
    return implicitParams[n]->getType();
}

int Signature::findImplicitParam(Exp *e) {
    for (int i = 0; i < getNumImplicitParams(); i++)
        if (*getImplicitParamExp(i) == *e)
            return i;
    return -1;
}

int Signature::findReturn(Exp *e) {
    for (int i = 0; i < getNumReturns(); i++)
        if (*getReturnExp(i) == *e)
            return i;
    return -1;
}

void Signature::addReturn(Type *type, Exp *exp) {
    assert(exp);
    addReturn(new Return(type, exp));
}

void Signature::addReturn(Exp *exp) {
    addReturn(new IntegerType(), exp);
}

void Signature::removeReturn(Exp *e)
{
    int i = findReturn(e);
    if (i != -1) {
        for (unsigned j = i+1; j < returns.size(); j++)
            returns[j-1] = returns[j];
        returns.resize(returns.size()-1);
    }
}

int Signature::getNumReturns() {
    return returns.size();
}

Exp *Signature::getReturnExp(int n) {
    return returns[n]->getExp();
}

void Signature::setReturnExp(int n, Exp* e) {
    returns[n]->setExp(e);
}

Type *Signature::getReturnType(int n) {
    return returns[n]->getType();
}

void Signature::setReturnType(int n, Type *ty) {
    returns[n]->setType(ty);
}

void Signature::fixReturnsWithParameters() {
    for (unsigned i = 0; i < params.size(); i++) { 
        int n = returns.size();
        for (int j=0; j < n; j++) {
            bool change;
            RefExp r(getParamExp(i)->clone(), NULL);
            Exp*& retExp = returns[j]->getRefExp();
            retExp = retExp->searchReplaceAll(&r, 
                                Location::param(getParamName(i)), change);
        }
    }
}

Exp *Signature::getArgumentExp(int n) {
    return getParamExp(n);
}

Signature *Signature::promote(UserProc *p) {
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

    return this;
}

Signature *Signature::instantiate(platform plat, callconv cc, const char *nam) {
    switch (plat) {
        case PLAT_PENTIUM:
            if (cc == CONV_PASCAL)
                // For now, assume the only pascal calling convention pentium
                // signatures will be Windows
                return new CallingConvention::Win32Signature(nam);
            else
                return new CallingConvention::StdC::PentiumSignature(nam);
        case PLAT_SPARC:
            assert(cc == CONV_C);
            return new CallingConvention::StdC::SparcSignature(nam);
        // insert other conventions here
        default:
            std::cerr << "unknown signature: " << conventionName(cc) << " " <<
                platformName(plat) << "\n";
            assert(false);
    }
    return NULL;
}

void Signature::print(std::ostream &out)
{
    if (returns.size() >= 1)
        out << returns[0]->getType()->getCtype() << " ";
    else
        out << "void ";
    out << name << "(";
    for (unsigned i = 0; i < params.size(); i++) {
        out << params[i]->getType()->getCtype() << " " << params[i]->getName() << " " << params[i]->getExp();
        if (i != params.size()-1) out << ", ";
    }
    out << "   implicit: ";
    for (unsigned i = 0; i < implicitParams.size(); i++) {
        out << implicitParams[i]->getType()->getCtype() << " " << implicitParams[i]->getName() << " " 
            << implicitParams[i]->getExp();
        if (i != implicitParams.size()-1) out << ", ";
    }
    out << ") { "; 
    for (unsigned i = 0; i < returns.size(); i++) {
        out << returns[i]->getExp();
        if (i != returns.size()-1) out << ", ";
    }
    out << " }" << std::endl;
}

void Signature::printToLog()
{
    std::ostringstream os;
    print(os);
    LOG << os.str().c_str();
}

void Signature::getInternalStatements(StatementList &stmts)
{
}

// No longer used; may be used again in future
#if 0
void Signature::analyse(UserProc *p) {
    if (VERBOSE) {
        std::cerr << "accepted promotion" << std::endl;
        std::cerr << "searching for creation of return value" << std::endl;
    }
    StatementList internal;
    //p->getInternalStatements(internal);
    StmtListIter it;
    for (Statement* s = internal.getFirst(it); s; s = internal.getNext(it)) {
        if (s->getLeft() && *s->getLeft() == *getReturnExp() &&
            s->getRight() && !(*s->getLeft() == *s->getRight())) {
            if (VERBOSE) {
                std::cerr << "found: ";
                s->printAsUse(std::cerr);
                std::cerr << std::endl;
            }
            //p->eraseInternalStatement(s);
            p->getCFG()->setReturnVal(s->getRight()->clone());
            updateParams(p, s);
            setReturnType(new IntegerType());
        }
    }
    StmtSetIter ll;
    StatementSet& lout = *p->getCFG()->getReachExit();
    for (Statement* s = lout.getFirst(ll); s; s = lout.getNext(ll)) {
        if (s->getLeft() && *s->getLeft() == *getReturnExp()) {
            if (VERBOSE) {
                std::cerr << "found: ";
                s->printAsUse(std::cerr);
                std::cerr << std::endl;
            }
            p->getCFG()->setReturnVal(s->getLeft()->clone());
            CallStatement *call = dynamic_cast<CallStatement*>(s);
            Type *ty = NULL;
            if (call)
                ty = call->getLeftType();
            if (call && ty)
                setReturnType(ty->clone());
            else
                setReturnType(new IntegerType());
        }
    }
    if (VERBOSE)
        std::cerr << "searching for parameters in statements" << std::endl;
    StatementList stmts;
    p->getStatements(stmts);
    StmtListIter si;
    for (Statement* s = stmts.getFirst(si); s; s = stmts.getNext(si)) {
        if (VERBOSE) std::cerr << "updateParameters for " << s << std::endl;
        updateParams(p, s);
    }
/*    std::cerr << "searching for parameters in internals" << std::endl;
    internal.clear();
    p->getInternalStatements(internal);
    for (Statement* s = internal.getFirst(it); s; s = internal.getNext(it)) {
    updateParams(p, s, false); */
}
#endif

// Note: the below few functions require reaching definitions.
// Likely can't be used
void Signature::updateParams(UserProc *p, Statement *stmt, bool checkreach) {
    int i;
    if (usesNewParam(p, stmt, checkreach, i)) {
        int n = getNumParams();
        setNumParams(i+1);
        for (n = 0; n < getNumParams(); n++) {
            if (VERBOSE) std::cerr << "found param " << n << std::endl;
            p->getCFG()->searchAndReplace(getParamExp(n), 
                Location::param(getParamName(n)));
        }
    }
}

bool Signature::usesNewParam(UserProc *p, Statement *stmt, bool checkreach,
  int &n) {
    n = getNumParams() - 1;
    if (VERBOSE) {
        std::cerr << "searching ";
        stmt->printAsUse(std::cerr);
        std::cerr << std::endl;
    }
    StatementSet reachin;
    //stmt->getReachIn(reachin, 2);
    for (int i = getNumParams(); i < 10; i++)
        if (stmt->usesExp(getParamExp(i))) {
            bool ok = true;
            if (checkreach) {
                bool hasDef = false;
                    StatementSet::iterator it1;
                    for (it1 = reachin.begin(); it1 != reachin.end(); it1++) {
                        if (*(*it1)->getLeft() == *getParamExp(i)) {
                            hasDef = true; break; 
                        }
                    }
                    if (hasDef) ok = false;
            }
            if (ok) {
                n = i;
            }
        }
    return n > (getNumParams() - 1);
}

void Signature::addImplicitParametersFor(Parameter *pn)
{
    Type *type = pn->getType();
    Exp *e = pn->getExp();
    if (type && type->resolvesToPointer()) { 
        PointerType *p = type->asPointer();
        /* seems right, if you're passing a pointer to a procedure
         * then that procedure probably uses what the pointer points
         * to.  Need to add them as arguments so SSA finds em.
         */
        Type *points_to = p->getPointsTo();
        if (points_to->resolvesToCompound()) {
            CompoundType *c = (CompoundType*)points_to;
            int base = 0;
            for (int n = 0; n < c->getNumTypes(); n++) {
                Exp *e1 = Location::memOf(new Binary(opPlus, e->clone(),
                                    new Const(base / 8)));
                e1 = e1->simplify();
                addImplicitParameter(c->getType(n), c->getName(n), e1, pn);
                base += c->getType(n)->getSize();
            }
        } else if (!points_to->resolvesToFunc()) 
            addImplicitParameter(points_to, NULL, 
                              Location::memOf(e->clone()), pn);
    }
}

void Signature::addImplicitParameter(Type *type, const char *nam, Exp *e, Parameter *parent)
{
    if (nam == NULL) {
        std::ostringstream os;
        os << "implicit" << implicitParams.size();
        nam = os.str().c_str();
    }
    ImplicitParameter *p = new ImplicitParameter(type, nam, e, parent);
    implicitParams.push_back(p);
    addImplicitParametersFor(p);
}

void Signature::addImplicitParameter(Exp *e)
{
    addImplicitParameter(new IntegerType(), NULL, e, NULL);
}

void Signature::removeImplicitParameter(int i)
{
    for (unsigned j = i+1; j < implicitParams.size(); j++)
        implicitParams[j-1] = implicitParams[j];
    implicitParams.resize(implicitParams.size()-1);
}

// Special for Mike: find the location where the first outgoing (actual)
// parameter is conventionally held
Exp* Signature::getFirstArgLoc(Prog* prog) {
    MACHINE mach = prog->getMachine();
    switch (mach) {
        case MACHINE_SPARC: {
            CallingConvention::StdC::SparcSignature sig("");
            return sig.getArgumentExp(0);
        }
        case MACHINE_PENTIUM: {
            //CallingConvention::StdC::PentiumSignature sig("");
            //Exp* e = sig.getArgumentExp(0);
            // For now, need to work around how the above appears to be the
            // wrong thing!
Exp* e = Location::memOf(Location::regOf(28));
            return e;
        }
        default:
            std::cerr << "Signature::getFirstArgLoc: machine not handled\n";
            assert(0);
    }
    return 0;
}

// A bit of a cludge. Problem is that we can't call the polymorphic
// getReturnExp() until signature promotion has happened. For the switch
// logic, that happens way too late. So for now, we have this cludge.
// This is very very hacky! (trent)
/*static*/ Exp* Signature::getReturnExp2(BinaryFile* pBF) {
    switch (pBF->GetMachine()) {
        case MACHINE_SPARC: 
            return Location::regOf(8);
        case MACHINE_PENTIUM:
            return Location::regOf(24);
        default:
            std::cerr << "getReturnExp2: machine not handled\n";
            return NULL;
    }
    return NULL;
}

// Not very satisfying to do things this way. Problem is that the polymorphic
// CallingConvention objects are set up very late in the decompilation
// Get the set of registers that are not saved in library functions (or any
// procedures that follow the calling convention)
// Caller is to delete the list (unless NULL, of course)
std::list<Exp*> *Signature::getCallerSave(Prog* prog) {
    MACHINE mach = prog->getMachine();
    switch (mach) {
        case MACHINE_PENTIUM: {
            std::list<Exp*> *li = new std::list<Exp*>;
            li->push_back(Location::regOf(24));    // eax
            li->push_back(Location::regOf(25));    // ecx
            li->push_back(Location::regOf(26));    // edx
            return li;
        }
        case MACHINE_SPARC: {
            std::list<Exp*> *li = new std::list<Exp*>;
            li->push_back(Location::regOf(8));    // %o0
            li->push_back(Location::regOf(9));    // %o1
            li->push_back(Location::regOf(10));   // %o2
            li->push_back(Location::regOf(11));   // %o3
            li->push_back(Location::regOf(12));   // %o4
            li->push_back(Location::regOf(13));   // %o5
            li->push_back(Location::regOf(1));    // %g1
            return li;
        }
        default:
            break;
    }
    return NULL;
}

// Get the expected argument location, based solely on the machine of the
// input program
Exp* Signature::getEarlyParamExp(int n, Prog* prog) {
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
        default:
            break;
    }
    assert(0);          // Machine not handled
    return NULL;
}

StatementList& Signature::getStdRetStmt(Prog* prog) {
    // pc := m[r[28]]
    static Assign pent1ret(
        new Terminal(opPC),
        Location::memOf(Location::regOf(28)));
    // r[28] := r[28] + 4
    static Assign pent2ret(
        Location::regOf(28),
        new Binary(opPlus,
            Location::regOf(28),
            new Const(4)));
    MACHINE mach = prog->getMachine();
    switch (mach) {
        case MACHINE_SPARC:
            break;              // No adjustment to stack pointer required
        case MACHINE_PENTIUM: {
            StatementList* sl = new StatementList;
            sl->append((Statement*)&pent1ret);
            sl->append((Statement*)&pent2ret);
            return *sl;
        }
        default:
            break;
    }
    return *new StatementList;
}

// Temporary hack till get signature promotion sorted
int Signature::getStackRegister(Prog* prog) {
    MACHINE mach = prog->getMachine();
    switch (mach) {
        case MACHINE_SPARC:
            return 14;
        case MACHINE_PENTIUM:
            return 28;
        default:
            return 0;
    }
}

bool Signature::isStackLocal(Prog* prog, Exp *e)
{
    static Exp *sp = Location::regOf(getStackRegister(prog));
    return e->getOper() == opMemOf && 
           e->getSubExp1()->getOper() == opMinus &&
           *e->getSubExp1()->getSubExp1() == *sp &&
           e->getSubExp1()->getSubExp2()->getOper() == opIntConst;
}


bool Parameter::operator==(Parameter& other) {
    if (!(*type == *other.type)) return false;
    // Do we really care about a parameter's name?
    if (!(name == other.name)) return false;
    if (!(*exp == *other.exp)) return false;
    return true;
}

bool Return::operator==(Return& other) {
    if (!(*type == *other.type)) return false;
    if (!(*exp == *other.exp)) return false;
    return true;
}
    
