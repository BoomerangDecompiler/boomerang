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
#include "dataflow.h"
#include "exp.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "signature.h"
#include "util.h"
#include "cfg.h"
#include "proc.h"
#include "boomerang.h"

#define VERBOSE Boomerang::get()->vFlag

namespace CallingConvention {

    class Win32Signature : public Signature {
    public:
        Win32Signature(const char *nam);
        Win32Signature(Signature &old);
        virtual ~Win32Signature() { }
        virtual Signature *clone();
        virtual bool operator==(const Signature& other) const;
        static bool qualified(UserProc *p, Signature &candidate);

        virtual bool serialize(std::ostream &ouf, int len);
        virtual bool deserialize_fid(std::istream &inf, int fid);

        virtual Exp *getReturnExp();

        virtual Exp *getParamExp(int n);
        virtual Exp *getArgumentExp(int n);

        virtual Signature *promote(UserProc *p);
        virtual void getInternalStatements(std::list<Statement*> &stmts);
	virtual Exp *getStackWildcard();
    };

    namespace StdC {
        class PentiumSignature : public Signature {
        public:
            PentiumSignature(const char *nam);
            PentiumSignature(Signature &old);
            virtual ~PentiumSignature() { }
            virtual Signature *clone(); 
            virtual bool operator==(const Signature& other) const;
            static bool qualified(UserProc *p, Signature &candidate);

            virtual bool serialize(std::ostream &ouf, int len);
            virtual bool deserialize_fid(std::istream &inf, int fid);

            virtual Exp *getReturnExp();

            virtual Exp *getParamExp(int n);
            virtual Exp *getArgumentExp(int n);

            virtual Signature *promote(UserProc *p);
            virtual void getInternalStatements(std::list<Statement*> &stmts);
	    virtual Exp *getStackWildcard();
        };      

        class SparcSignature : public Signature {
        public:
            SparcSignature(const char *nam);
            SparcSignature(Signature &old);
            virtual ~SparcSignature() { }
            virtual Signature *clone();
            virtual bool operator==(const Signature& other) const;
            static bool qualified(UserProc *p, Signature &candidate);

            virtual bool serialize(std::ostream &ouf, int len);
            virtual bool deserialize_fid(std::istream &inf, int fid);

            virtual Exp *getReturnExp();

            virtual Exp *getParamExp(int n);
            virtual Exp *getArgumentExp(int n);

            virtual Signature *promote(UserProc *p);
	    virtual Exp *getStackWildcard();
        };
    };
};

CallingConvention::Win32Signature::Win32Signature(const char *nam) : Signature(nam)
{

}

CallingConvention::Win32Signature::Win32Signature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::Win32Signature::clone()
{
    Win32Signature *n = new Win32Signature(name.c_str());
    n->params = params;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

bool CallingConvention::Win32Signature::operator==(const Signature& other) const
{
    // TODO
    return false;
}

bool CallingConvention::Win32Signature::qualified(UserProc *p, Signature &candidate)
{
    std::string feid(p->getProg()->getFrontEndId());
    if (feid != "pentium" || !p->getProg()->isWin32()) return false;

    if (VERBOSE) {
        std::cerr << "consider promotion to stdc win32 signature for " << p->getName() << std::endl;
    }

    bool gotcorrectret1 = false;
    bool gotcorrectret2 = false;
    std::list<Statement*> internal;
    p->getInternalStatements(internal);
    for (std::list<Statement*>::iterator it = internal.begin();
         it != internal.end(); it++) {
        AssignExp *e = dynamic_cast<AssignExp*>(*it);
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
                ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt() == 28) {
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
                ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt() == 28 &&
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

bool CallingConvention::Win32Signature::serialize(std::ostream &ouf, int len)
{
    std::streampos st = ouf.tellp();

    char type = 0;
    saveValue(ouf, type, false);
    saveString(ouf, name);

    for (unsigned i = 0; i < params.size(); i++) {
        saveFID(ouf, FID_SIGNATURE_PARAM);
        std::streampos pos = ouf.tellp();
        int len = -1;
        saveLen(ouf, -1, true);
        std::streampos posa = ouf.tellp();

        saveString(ouf, params[i]->getName());
        int l;
        params[i]->getType()->serialize(ouf, l);

        std::streampos now = ouf.tellp();
        assert((int)(now - posa) == len);
        ouf.seekp(pos);
        saveLen(ouf, len, true);
        ouf.seekp(now);
    }

    saveFID(ouf, FID_SIGNATURE_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;    
}

bool CallingConvention::Win32Signature::deserialize_fid(std::istream &inf, int fid)
{
    switch(fid) {
        case FID_SIGNATURE_PARAM:
            {
                std::streampos pos = inf.tellg();
                int len = loadLen(inf);
                std::string nam;
                loadString(inf, nam);
                Type *t = Type::deserialize(inf);
                params.push_back(new Parameter(t, nam.c_str()));
                std::streampos now = inf.tellg();
                assert(len == (now - pos));
            }
            break;
        default:
            return Signature::deserialize_fid(inf, fid);
    }
    return true;
}

Exp *CallingConvention::Win32Signature::getReturnExp()
{   
    return new Unary(opRegOf, new Const(24));
}

Exp *CallingConvention::Win32Signature::getParamExp(int n)
{
    Exp *esp = new Unary(opRegOf, new Const(28));
    return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

Exp *CallingConvention::Win32Signature::getArgumentExp(int n)
{
    Exp *esp = new Unary(opRegOf, new Const(28));
    return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

Signature *CallingConvention::Win32Signature::promote(UserProc *p)
{
    // no promotions from win32 signature up, yet.
    // a possible thing to investigate would be COM objects
    return this;
}

Exp *CallingConvention::Win32Signature::getStackWildcard()
{
    return new Unary(opMemOf, new Binary(opPlus, new Unary(opRegOf, 
               new Const(28)), new Terminal(opWild)));
}

void CallingConvention::Win32Signature::getInternalStatements(std::list<Statement*> &stmts)
{
    static AssignExp *fixpc = new AssignExp(new Terminal(opPC),
            new Unary(opMemOf, new Unary(opRegOf, new Const(28))));
    static AssignExp *fixesp = new AssignExp(new Unary(opRegOf, new Const(28)),
            new Binary(opPlus, new Unary(opRegOf, new Const(28)),
                new Const(4 + params.size()*4)));
    stmts.push_back((AssignExp*)fixpc->clone());
    stmts.push_back((AssignExp*)fixesp->clone());
}

CallingConvention::StdC::PentiumSignature::PentiumSignature(const char *nam) : Signature(nam)
{

}

CallingConvention::StdC::PentiumSignature::PentiumSignature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::StdC::PentiumSignature::clone()
{
    PentiumSignature *n = new PentiumSignature(name.c_str());
    n->params = params;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

bool CallingConvention::StdC::PentiumSignature::operator==(const Signature& other) const
{
    // TODO
    return false;
}


bool CallingConvention::StdC::PentiumSignature::qualified(UserProc *p, Signature &candidate)
{
    std::string feid(p->getProg()->getFrontEndId());
    if (feid != "pentium") return false;

    if (VERBOSE)
        std::cerr << "consider promotion to stdc pentium signature for " << p->getName() << std::endl;

    bool gotcorrectret1 = false;
    bool gotcorrectret2 = false;
    std::list<Statement*> internal;
    p->getInternalStatements(internal);
    for (std::list<Statement*>::iterator it1 = 
            p->getCFG()->getLiveOut().begin(); 
         it1 != p->getCFG()->getLiveOut().end(); it1++)
        internal.push_back(*it1);
    for (std::list<Statement*>::iterator it = internal.begin();
         it != internal.end(); it++) {
        AssignExp *e = dynamic_cast<AssignExp*>(*it);
        if (e == NULL) continue;
        if (e->getLeft()->getOper() == opPC) {
            if (e->getRight()->isMemOf() && 
                e->getRight()->getSubExp1()->isRegOf() &&
                e->getRight()->getSubExp1()->getSubExp1()->isIntConst() &&
                ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt() == 28) {
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
                ((Const*)e->getRight()->getSubExp1()->getSubExp1())->getInt() == 28 &&
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

bool CallingConvention::StdC::PentiumSignature::serialize(std::ostream &ouf, int len)
{
    std::streampos st = ouf.tellp();

    char type = 0;
    saveValue(ouf, type, false);
    saveString(ouf, name);

    for (unsigned i = 0; i < params.size(); i++) {
        saveFID(ouf, FID_SIGNATURE_PARAM);
        std::streampos pos = ouf.tellp();
        int len = -1;
        saveLen(ouf, -1, true);
        std::streampos posa = ouf.tellp();

        saveString(ouf, params[i]->getName());
        int l;
        params[i]->getType()->serialize(ouf, l);

        std::streampos now = ouf.tellp();
        assert((int)(now - posa) == len);
        ouf.seekp(pos);
        saveLen(ouf, len, true);
        ouf.seekp(now);
    }

    saveFID(ouf, FID_SIGNATURE_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;    
}

bool CallingConvention::StdC::PentiumSignature::deserialize_fid(std::istream &inf, int fid)
{
    switch(fid) {
        case FID_SIGNATURE_PARAM:
            {
                std::streampos pos = inf.tellg();
                int len = loadLen(inf);
                std::string nam;
                loadString(inf, nam);
                Type *t = Type::deserialize(inf);
                params.push_back(new Parameter(t, nam.c_str()));
                std::streampos now = inf.tellg();
                assert(len == (now - pos));
            }
            break;
        default:
            return Signature::deserialize_fid(inf, fid);
    }
    return true;
}

Exp *CallingConvention::StdC::PentiumSignature::getReturnExp()
{
    return new Unary(opRegOf, new Const(24));
}

Exp *CallingConvention::StdC::PentiumSignature::getParamExp(int n)
{
    Exp *esp = new Unary(opRegOf, new Const(28));
    return new Unary(opMemOf, new Binary(opPlus, esp, new Const((int)((n+1) * 4))));
}

Exp *CallingConvention::StdC::PentiumSignature::getArgumentExp(int n)
{
    Exp *esp = new Unary(opRegOf, new Const(28));
        //if (n == 0)
    //    return new Unary(opMemOf, esp);
    return new Unary(opMemOf, new Binary(opPlus, esp, 
                new Const((int)((n+1) * 4))));
}

Signature *CallingConvention::StdC::PentiumSignature::promote(UserProc *p)
{
    // No promotions from here up, obvious idea would be c++ name mangling  
    return this;
}

Exp *CallingConvention::StdC::PentiumSignature::getStackWildcard()
{
    return new Unary(opMemOf, new Binary(opPlus, new Unary(opRegOf, 
               new Const(28)), new Terminal(opWild)));
}


void CallingConvention::StdC::PentiumSignature::getInternalStatements(std::list<Statement*> &stmts)
{
    static AssignExp *fixpc = new AssignExp(new Terminal(opPC),
            new Unary(opMemOf, new Unary(opRegOf, new Const(28))));
    static AssignExp *fixesp = new AssignExp(new Unary(opRegOf, new Const(28)),
            new Binary(opPlus, new Unary(opRegOf, new Const(28)),
                new Const(4)));
    stmts.push_back((AssignExp*)fixpc->clone());
    stmts.push_back((AssignExp*)fixesp->clone());
}

CallingConvention::StdC::SparcSignature::SparcSignature(const char *nam) : Signature(nam)
{

}

CallingConvention::StdC::SparcSignature::SparcSignature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::StdC::SparcSignature::clone()
{
    SparcSignature *n = new SparcSignature(name.c_str());
    n->params = params;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

bool CallingConvention::StdC::SparcSignature::operator==(const Signature& other) const
{
    // TODO
    return false;
}

bool CallingConvention::StdC::SparcSignature::qualified(UserProc *p, Signature &candidate)
{
    std::string feid(p->getProg()->getFrontEndId());
    if (feid != "sparc") return false;

    // is there other constraints?
    
    return true;
}

bool CallingConvention::StdC::SparcSignature::serialize(std::ostream &ouf, int len)
{
    std::streampos st = ouf.tellp();

    char type = 0;
    saveValue(ouf, type, false);
    saveString(ouf, name);

    for (unsigned i = 0; i < params.size(); i++) {
        saveFID(ouf, FID_SIGNATURE_PARAM);
        std::streampos pos = ouf.tellp();
        int len = -1;
        saveLen(ouf, -1, true);
        std::streampos posa = ouf.tellp();

        saveString(ouf, params[i]->getName());
        int l;
        params[i]->getType()->serialize(ouf, l);

        std::streampos now = ouf.tellp();
        assert((int)(now - posa) == len);
        ouf.seekp(pos);
        saveLen(ouf, len, true);
        ouf.seekp(now);
    }

    saveFID(ouf, FID_SIGNATURE_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;    
}

bool CallingConvention::StdC::SparcSignature::deserialize_fid(std::istream &inf, int fid)
{
    switch(fid) {
        case FID_SIGNATURE_PARAM:
            {
                std::streampos pos = inf.tellg();
                int len = loadLen(inf);
                std::string nam;
                loadString(inf, nam);
                Type *t = Type::deserialize(inf);
                params.push_back(new Parameter(t, nam.c_str()));
                std::streampos now = inf.tellg();
                assert(len == (now - pos));
            }
            break;
        default:
            return Signature::deserialize_fid(inf, fid);
    }
    return true;
}

Exp *CallingConvention::StdC::SparcSignature::getReturnExp()
{
    // MVE: Note that doubles are returned in f0:f1
    // So how do we say that?
    // When structs are returned, the size appears after the end of the
    // function
    // For most things, the return value ends up in %o0, from the caller's
    // perspective. For most callees (with save/restore), the actual assign-
    // ment will be to %i0 (register 24), and the restore will copy %i0 to %o0.
    return new Unary(opRegOf, new Const(8));
}

Exp *CallingConvention::StdC::SparcSignature::getParamExp(int n)
{
    // Note: although it looks like actual parameters are in %i0, %i1 etc in
    // most SPARC procedures, this is a result of the semantics of the commonly
    // seen (but not essential) SAVE instruction. So in reality, both formal
    // and actual parameters are seen in %o0, %o1, ... at the start of the
    // procedure
    // return new Unary(opRegOf, new Const((int)(24 + n)));
    if (n >= 6) {
        // SPARCs pass the seventh and subsequent parameters at m[%sp+92],
        // m[%esp+96], etc.
        return new Unary(opMemOf,
            new Binary(opPlus,
                new Unary(opRegOf, new Const(14)),      // %o6 == %sp
                new Const(92 + (n-6)*4)));
    }
    return new Unary(opRegOf, new Const((int)(8 + n)));
}

Exp *CallingConvention::StdC::SparcSignature::getArgumentExp(int n)
{
    if (n >= 6) {
        // SPARCs pass the seventh and subsequent parameters at m[%sp+92],
        // m[%esp+96], etc.
        return new Unary(opMemOf,
            new Binary(opPlus,
                new Unary(opRegOf, new Const(14)),      // %o6 == %sp
                new Const(92 + (n-6)*4)));
    }
    return new Unary(opRegOf, new Const((int)(8 + n)));
}

Signature *CallingConvention::StdC::SparcSignature::promote(UserProc *p)
{
    // no promotions from here up, obvious example would be name mangling
    return this;
}

Exp *CallingConvention::StdC::SparcSignature::getStackWildcard()
{
    return new Unary(opMemOf, new Binary(opPlus, new Unary(opRegOf, 
               new Const(14)), new Terminal(opWild)));
}

Signature::Signature(const char *nam) : rettype(new VoidType()), ellipsis(false)
{
    name = nam;
}

Signature *Signature::clone()
{
    Signature *n = new Signature(name.c_str());
    n->params = params;
    n->ellipsis = ellipsis;
    n->rettype = rettype;
    return n;
}

bool Signature::operator==(const Signature& other) const
{
    // TODO
    return false;
}

bool Signature::serialize(std::ostream &ouf, int len)
{
    std::streampos st = ouf.tellp();

    char type = 0;
    saveValue(ouf, type, false);
    saveString(ouf, name);

    for (unsigned i = 0; i < params.size(); i++) {
        saveFID(ouf, FID_SIGNATURE_PARAM);
        std::streampos pos = ouf.tellp();
        int len = -1;
        saveLen(ouf, -1, true);
        std::streampos posa = ouf.tellp();

        saveString(ouf, params[i]->getName());
        int l;
        params[i]->getType()->serialize(ouf, l);

        std::streampos now = ouf.tellp();
        assert((int)(now - posa) == len);
        ouf.seekp(pos);
        saveLen(ouf, len, true);
        ouf.seekp(now);
    }

    saveFID(ouf, FID_SIGNATURE_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

Signature *Signature::deserialize(std::istream &inf)
{
    Signature *sig = NULL;

    char type;
    loadValue(inf, type, false);
    assert(type == 0 || type == 1 || type == 2 || type == 3);

    std::string nam;
    loadString(inf, nam);

    switch(type) {
        case 0:
            sig = new Signature(nam.c_str());
            break;
        case 1:
            sig = new CallingConvention::Win32Signature(nam.c_str());
            break;
        case 2:
            sig = new CallingConvention::StdC::PentiumSignature(nam.c_str());
            break;
        case 3:
            sig = new CallingConvention::StdC::SparcSignature(nam.c_str());
            break;
    }
    assert(sig);
    
    int fid;
    while ((fid = loadFID(inf)) != -1 && fid != FID_SIGNATURE_END)
        sig->deserialize_fid(inf, fid);
    assert(loadLen(inf) == 0);

    return sig;
}

bool Signature::deserialize_fid(std::istream &inf, int fid)
{
    switch(fid) {
        case FID_SIGNATURE_PARAM:
            {
                std::streampos pos = inf.tellg();
                int len = loadLen(inf);
                std::string nam;
                loadString(inf, nam);
                Type *t = Type::deserialize(inf);
                params.push_back(new Parameter(t, nam.c_str()));
                std::streampos now = inf.tellg();
                assert(len == (now - pos));
            }
            break;
        default:
            return Signature::deserialize_fid(inf, fid);
    }
    return true;
}

Exp *Signature::getReturnExp()
{
    return NULL;
}

Type *Signature::getReturnType()
{
    return rettype;
}

void Signature::setReturnType(Type *t)
{
    if (rettype) delete rettype;
    rettype = t;
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

void Signature::addParameter(Type *type, const char *nam /*= NULL*/)
{
    std::string s;
    if (nam == NULL) {
        std::stringstream os;
        os << "arg" << params.size()+1 << std::ends;
        s = os.str();
        nam = s.c_str();
    }
    addParameter(new Parameter(type, nam));
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

int Signature::getNumParams()
{
    return params.size();
}

const char *Signature::getParamName(int n)
{
    assert(n < (int)params.size());
    return params[n]->getName();
}

Exp *Signature::getParamExp(int n)
{
    assert(false);
}

Type *Signature::getParamType(int n)
{
    static IntegerType def;
    assert(n < (int)params.size() || ellipsis);
    if (n >= (int)params.size()) return &def;
    return params[n]->getType();
}

Exp *Signature::getArgumentExp(int n)
{
    // TODO: esp?
    return getParamExp(n);
}

Signature *Signature::promote(UserProc *p)
{
    if (CallingConvention::Win32Signature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::Win32Signature(*this);
        sig->analyse(p);
        delete this;
        return sig;
    }

    if (CallingConvention::StdC::PentiumSignature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::StdC::PentiumSignature(*this);
        sig->analyse(p);
        delete this;
        return sig;
    }

    if (CallingConvention::StdC::SparcSignature::qualified(p, *this)) {
        Signature *sig = new CallingConvention::StdC::SparcSignature(*this);
        sig->analyse(p);
        delete this;
        return sig;
    }

    return this;
}

Signature *Signature::instantiate(const char *str, const char *nam)
{
    std::string s = str;
    if (s == "-win32-pentium") {
        return new CallingConvention::Win32Signature(nam);
    }
    if (s == "-stdc") {
        // need platform too
        assert(false);
    }
    if (s == "-stdc-pentium") {
        return new CallingConvention::StdC::PentiumSignature(nam);
    }
    if (s == "-stdc-sparc") {
        return new CallingConvention::StdC::SparcSignature(nam);
    }
    std::cerr << "unknown signature: " << s << std::endl;
    // insert other conventions here
    assert(false);
    return NULL;
}

void Signature::print(std::ostream &out)
{
    out << rettype->getCtype() << " " << name << "(";
    for (unsigned i = 0; i < params.size(); i++) {
        out << params[i]->getType()->getCtype() << " " << params[i]->getName();
        if (i != params.size()-1) out << ", ";
    }
    out << ")" << std::endl;
}

void Signature::getInternalStatements(std::list<Statement*> &stmts)
{
}

void Signature::analyse(UserProc *p)
{
    if (VERBOSE) {
        std::cerr << "accepted promotion" << std::endl;
        std::cerr << "searching for creation of return value" << std::endl;
    }
    std::list<Statement*> internal;
    p->getInternalStatements(internal);
    for (std::list<Statement*>::iterator it = internal.begin();
         it != internal.end(); it++) {
        if ((*it)->getLeft() && *(*it)->getLeft() == *getReturnExp() &&
            (*it)->getRight() && !(*(*it)->getLeft() == *(*it)->getRight())) {
            if (VERBOSE) {
                std::cerr << "found: ";
                (*it)->printAsUse(std::cerr);
                std::cerr << std::endl;
            }
            p->eraseInternalStatement(*it);
            p->getCFG()->setReturnVal((*it)->getRight()->clone());
            setReturnType(new IntegerType());
        }
    }
    for (std::set<Statement*>::iterator it = p->getCFG()->getLiveOut().begin();
         it != p->getCFG()->getLiveOut().end(); it++) {
        if ((*it)->getLeft() && *(*it)->getLeft() == *getReturnExp()) {
            if (VERBOSE) {
                std::cerr << "found: ";
                (*it)->printAsUse(std::cerr);
                std::cerr << std::endl;
            }
            p->getCFG()->setReturnVal((*it)->getLeft()->clone());
            HLCall *call = dynamic_cast<HLCall*>(*it);
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
        std::cerr << "searching for arguments in statements" << std::endl;
    std::set<Statement*> stmts;
    p->getStatements(stmts);
    for (std::set<Statement*>::iterator it = stmts.begin();
     it != stmts.end(); it++)
    updateParams(p, *it);
/*    std::cerr << "searching for arguments in internals" << std::endl;
    internal.clear();
    p->getInternalStatements(internal);
    for (std::list<Statement*>::iterator it = internal.begin();
         it != internal.end(); it++)
    updateParams(p, *it, false); */
}

void Signature::updateParams(UserProc *p, Statement *stmt, bool checklive)
{
    int i;
    if (usesNewParam(p, stmt, checklive, i)) {
        int n = getNumParams();
            setNumParams(i+1);
        for (; n < getNumParams(); n++) {
            if (VERBOSE) std::cerr << "found param " << n << std::endl;
            p->getCFG()->searchAndReplace(getParamExp(n), 
                new Unary(opParam, new Const((char *)getParamName(n))));
        }
    }
}

bool Signature::usesNewParam(UserProc *p, Statement *stmt, bool checklive,
  int &n) {
    n = getNumParams() - 1;
    if (VERBOSE) {
        std::cerr << "searching ";
        stmt->printAsUse(std::cerr);
        std::cerr << std::endl;
    }
    std::set<Statement*> livein;
    stmt->getLiveIn(livein);
    for (int i = getNumParams(); i < 10; i++)
        if (stmt->usesExp(getParamExp(i))) {
            bool ok = true;
            if (checklive) {
                bool hasDef = false;
                    for (std::set<Statement*>::iterator it1 = livein.begin();
                      it1 != livein.end(); it1++)
                        if (*(*it1)->getLeft() == *getParamExp(i)) {
                            hasDef = true; break; 
                        }
                    if (hasDef) ok = false;
            }
            if (ok) {
                n = i;
            }
        }
    return n > (getNumParams() - 1);
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
Exp* e = new Unary(opMemOf, new Unary(opRegOf, new Const(28)));
            return e;
        }
        default:
            std::cerr << "Signature::getFirstArgLoc: machine not handled\n";
            assert(0);
    }
    return 0;
}
