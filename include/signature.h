/*
 * Copyright (C) 2002, Trent Waddington
 */
/***************************************************************************//**
 * \file       signature.h
 * OVERVIEW:   Provides the definition for the signature classes.
 *============================================================================*/
/*
 * $Revision$    // 1.53.2.11
 *
 * 12 Jul 02 - Trent: Created
 *
 * Trent had the idea of "promoting" to signatures with known behaviour (e.g. conforms to ABI). However, it seems more
 *    general to only assume ABI behaviour for library functions, and derive the signature information from child
 *    procedures in all user procedures. At present, this promotion is basically disabled (promotion always succeeds,
 *    but not much is assumed by the process of promotion). The role of the Signature classes is still being considered.
 *    - MVE Jun 2005.
 */

#ifndef __SIGNATURE_H_
#define __SIGNATURE_H_

#if defined(_MSC_VER)
#pragma warning(disable:4290)
#endif

#include <string>
#include "exp.h"
#include "type.h"
#include "sigenum.h"        // For enums platform and cc
#include "memo.h"
#include "statement.h"        // For class Return

class Statement;
class StatementList;
class BinaryFile;
class XMLProgParser;

class Parameter {
private:
        Type *type;
        std::string name;
        Exp *exp;
        std::string boundMax;

public:
                    Parameter(Type *type, const char *name, Exp *exp = NULL, const char *boundMax = "") :
                    type(type), name(name), exp(exp), boundMax(boundMax)    { }
virtual                ~Parameter() { delete type; delete exp; }
        bool        operator==(Parameter& other);
        Parameter*    clone();

        Type        *getType() { return type; }
        void        setType(Type *ty) { type = ty; }
        const char    *getName() { return name.c_str(); }
        void        setName(const char *nam) { name = nam; }
        Exp            *getExp()        { return exp; }
        void        setExp(Exp *e) { exp = e; }

        // this parameter is the bound of another parameter with name nam
        const char  *getBoundMax() { return boundMax.c_str(); }
        void        setBoundMax(const char *nam);

protected:
        friend        class XMLProgParser;
                    Parameter() : type(NULL), name(""), exp(NULL) { }
};        // class Parameter

class Return {
public:
        Type        *type;
        Exp            *exp;

                    Return(Type *type, Exp *exp) : type(type), exp(exp) { }
virtual                ~Return() { }
        bool        operator==(Return& other);
        Return*        clone();

                    Return() : type(NULL), exp(NULL) { }
        friend class XMLProgParser;
};        // class Return

typedef std::vector<Return*> Returns;


class Signature {
protected:
        std::string     name;        // name of procedure
        std::string     sigFile;    // signature file this signature was read from (for libprocs)
        std::vector<Parameter*> params;
        // std::vector<ImplicitParameter*> implicitParams;
        Returns         returns;
        Type *          rettype;
        bool            ellipsis;
        bool            unknown;
        //bool        bFullSig;            // True if have a full signature from a signature file etc
        // True if the signature is forced with a -sf entry, or is otherwise known, e.g. WinMain
        bool            forced;
        Type *          preferedReturn;
        std::string     preferedName;
        std::vector<int> preferedParams;

//        void        updateParams(UserProc *p, Statement *stmt, bool checkreach = true);
        bool        usesNewParam(UserProc *p, Statement *stmt, bool checkreach, int &n);

        //void        addImplicitParametersFor(Parameter *p);
        //void        addImplicitParameter(Type *type, const char *name, Exp *e, Parameter *parent);

public:
                    Signature(const char *nam);
        // Platform plat, calling convention cc (both enums)
        // nam is name of the procedure (no longer stored in the Proc)
static    Signature    *instantiate(platform plat, callconv cc, const char *nam);
virtual                ~Signature() { }

virtual bool        operator==(Signature& other);

        // clone this signature
virtual    Signature    *clone();

        bool        isUnknown() { return unknown; }
        void        setUnknown(bool b) { unknown = b; }
//        void        setFullSig(bool full) {bFullSig = full;}
        bool        isForced() {return forced; }
        void        setForced(bool f) {forced = f; }

        // get the return location
virtual void        addReturn(Type *type, Exp *e = NULL);
virtual void        addReturn(Exp *e);
virtual void        addReturn(Return *ret) { returns.push_back(ret); }
virtual void        removeReturn(Exp *e);
virtual unsigned    getNumReturns() {return returns.size();}
virtual Exp *       getReturnExp(int n) {return returns[n]->exp;}
        void        setReturnExp(int n, Exp* e) {returns[n]->exp = e;}
virtual Type *      getReturnType(int n) {return returns[n]->type;}
virtual void        setReturnType(int n, Type *ty);
        int         findReturn(Exp *e);
//      void        fixReturnsWithParameters();            // Needs description
        void        setRetType(Type *t) { rettype = t; }
        Returns &   getReturns() {return returns;}
        Type *      getTypeFor(Exp* e);

        // get/set the name
virtual const char    *getName();
virtual void        setName(const char *nam);
        // get/set the signature file
        const char    *getSigFile() { return sigFile.c_str(); }
        void        setSigFile(const char *nam) { sigFile = nam; }

        // add a new parameter to this signature
virtual void        addParameter(const char *nam = NULL);
virtual void        addParameter(Type *type, const char *nam = NULL, Exp *e = NULL, const char *boundMax = "");
virtual void        addParameter(Exp *e, Type* ty);
virtual void        addParameter(Parameter *param);
        void        addEllipsis() { ellipsis = true; }
        void        killEllipsis() {ellipsis = false; }
virtual void        removeParameter(Exp *e);
virtual void        removeParameter(int i);
        // set the number of parameters using defaults
virtual void        setNumParams(int n);

        // accessors for parameters
virtual unsigned    getNumParams() {return params.size();}
virtual const char *getParamName(int n);
virtual Exp *       getParamExp(int n);
virtual Type *      getParamType(int n);
virtual const char *getParamBoundMax(int n);
virtual void        setParamType(int n, Type *ty);
virtual void        setParamType(const char* nam, Type *ty);
virtual void        setParamType(Exp* e, Type *ty);
virtual void        setParamName(int n, const char *nam);
virtual void        setParamExp(int n, Exp *e);
virtual int         findParam(Exp *e);
virtual int         findParam(const char *nam);
        // accessor for argument expressions
virtual Exp *       getArgumentExp(int n);
virtual bool        hasEllipsis() { return ellipsis; }

        void        renameParam(const char *oldName, const char *newName);

        // analysis determines parameters / return type
        //virtual void analyse(UserProc *p);

        // Data flow based type analysis. Meet the parameters with their current types.  Returns true if a change
        bool        dfaTypeAnalysis(Cfg* cfg);

        // any signature can be promoted to a higher level signature, if available
virtual Signature *promote(UserProc *p);
        void        print(std::ostream &out, bool html = false);
        char*        prints();            // For debugging
        void        printToLog();

        // Special for Mike: find the location that conventionally holds the first outgoing (actual) parameter
        // MVE: Use the below now
        Exp*        getFirstArgLoc(Prog* prog);

        // This is like getParamLoc, except that it works before Signature::analyse is called.  It is used only to order
        // parameters correctly, for the common case where the proc will end up using a standard calling convention
        Exp*        getEarlyParamExp(int n, Prog* prog);

        // Get a wildcard to find stack locations
virtual Exp            *getStackWildcard() { return NULL; }
    class StackRegisterNotDefinedException : public std::exception {
    public:
        StackRegisterNotDefinedException() { }
    };
virtual int            getStackRegister(            ) throw(StackRegisterNotDefinedException);
static    int            getStackRegister(Prog* prog) throw(StackRegisterNotDefinedException);
        // Does expression e represent a local stack-based variable?
        // Result can be ABI specific, e.g. sparc has locals in the parent's stack frame, at POSITIVE offsets from the
        // stack pointer register
        // Also, I believe that the PA/RISC stack grows away from 0
        bool        isStackLocal(Prog* prog, Exp *e);
        // Similar to the above, but checks for address of a local (i.e. sp{0} -/+ K)
virtual    bool        isAddrOfStackLocal(Prog* prog, Exp* e);
        // For most machines, local variables are always NEGATIVE offsets from sp
virtual bool        isLocalOffsetNegative() {return true;}
        // For most machines, local variables are not POSITIVE offsets from sp
virtual bool        isLocalOffsetPositive() {return false;}
        // Is this operator (between the stack pointer and a constant) compatible with a stack local pattern?
        bool        isOpCompatStackLocal(OPER op);

        // Quick and dirty hack
static    Exp*        getReturnExp2(BinaryFile* pBF);
static    StatementList& getStdRetStmt(Prog* prog);

        // get anything that can be proven as a result of the signature
virtual Exp            *getProven(Exp *left) { return NULL; }
virtual    bool        isPreserved(Exp* e) { return false; }        // Return whether e is preserved by this proc
virtual    void        setLibraryDefines(StatementList* defs) {}    // Set the locations defined by library calls
static    void        setABIdefines(Prog* prog, StatementList* defs);

        // Return true if this is a known machine (e.g. SparcSignature as opposed to Signature)
virtual bool        isPromoted() { return false; }
        // Return true if this has a full blown signature, e.g. main/WinMain etc.
        // Note that many calls to isFullSignature were incorrectly calls to isPromoted()
        //bool        isFullSignature() {return bFullSig;}

    // ascii versions of platform, calling convention name
static  const char*     platformName(platform plat);
static  const char*     conventionName(callconv cc);
virtual platform        getPlatform() { return PLAT_GENERIC; }
virtual callconv        getConvention() { return CONV_NONE; }

        // prefered format
        void        setPreferedReturn(Type *ty) { preferedReturn = ty; }
        void        setPreferedName(const char *nam) { preferedName = nam; }
        void        addPreferedParameter(int n) { preferedParams.push_back(n); }
        Type        *getPreferedReturn() { return preferedReturn; }
        const char    *getPreferedName() { return preferedName.c_str(); }
        unsigned int getNumPreferedParams() { return preferedParams.size(); }
        int            getPreferedParam(int n) { return preferedParams[n]; }

        // A compare function for arguments and returns. Used for sorting returns in calcReturn() etc
virtual    bool        argumentCompare(Assignment& a, Assignment& b);
virtual    bool        returnCompare(Assignment& a, Assignment& b);


protected:
        friend class XMLProgParser;
                    Signature() : name(""), rettype(NULL), ellipsis(false), preferedReturn(NULL), preferedName("") { }
        void        appendParameter(Parameter *p) { params.push_back(p); }
        //void        appendImplicitParameter(ImplicitParameter *p) { implicitParams.push_back(p); }
        void        appendReturn(Return *r) { returns.push_back(r); }
};    // class Signature

class CustomSignature : public Signature {
protected:
        int            sp;
public:
    CustomSignature(const char *nam);
virtual ~CustomSignature() { }
virtual    bool        isPromoted() { return true; }
virtual Signature    *clone();
        void        setSP(int nsp);
virtual int            getStackRegister() throw(StackRegisterNotDefinedException) {return sp; };
};

#endif
