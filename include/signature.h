/*
 * Copyright (C) 2002, Trent Waddington
 */
/*==============================================================================
 * FILE:       signature.h
 * OVERVIEW:   Provides the definition for the signature classes.
 *============================================================================*/
/*
 * $Revision$
 *
 * 12 Jul 02 - Trent: Created
 */

#ifndef __SIGNATURE_H_
#define __SIGNATURE_H_

#include "exp.h"
#include "type.h"
#include "sigenum.h"   // For enums platform and cc

class Statement;
class BinaryFile;

class Parameter { 
private:
    Type *type;
    std::string name;
    Exp *exp;

public: 
    Parameter(Type *type, const char *name, Exp *exp = NULL) : type(type), name(name), exp(exp)  { }
    ~Parameter() { delete type; delete exp; }

    Type *getType() { return type; }
    void setType(Type *ty) { type = ty; }
    const char *getName() { return name.c_str(); }
    Exp *getExp()       { return exp; }
    void setExp(Exp *e) { exp = e; }
};

class ImplicitParameter : public Parameter {
private:
    Parameter *parent;

public:
    ImplicitParameter(Type *type, const char *name, Exp *exp, Parameter *parent) : 
                            Parameter(type, name, exp), parent(parent) { }
    ~ImplicitParameter() { }

    Parameter *getParent() { return parent; }
};

class Return {
private:
    Type *type;
    Exp *exp;

public:
    Return(Type *type, Exp *exp) : type(type), exp(exp) { }
    ~Return() { delete type; delete exp; }

    Type *getType() { return type; }
    void setType(Type *ty) { type = ty; }
    Exp *getExp() { return exp; }
    Exp*& getRefExp() {return exp;}
    void setExp(Exp* e) { exp = e; }
};

class Signature {
protected:
    std::string name;       // name of procedure
    std::vector<Parameter*> params;
    std::vector<ImplicitParameter*> implicitParams;
    std::vector<Return*> returns;
    Type *rettype;
    bool ellipsis;

    void updateParams(UserProc *p, Statement *stmt, bool checkreach = true);
    bool usesNewParam(UserProc *p, Statement *stmt, bool checkreach, int &n);

    void addImplicitParametersFor(Parameter *p);
    void addImplicitParameter(Type *type, const char *name, Exp *e, Parameter *parent);

public:
    Signature(const char *nam);
    // Platform plat, calling convention cc (both enums)
    // nam is name of the procedure (no longer stored in the Proc)
    static Signature *instantiate(platform plat, callconv cc, const char *nam);
    virtual ~Signature() { }

    virtual bool operator==(const Signature& other) const;

    // clone this signature
    virtual Signature *clone();

    // get the return location
    virtual void addReturn(Type *type, Exp *e = NULL);
    virtual void addReturn(Exp *e);
    virtual void addReturn(Return *ret) { returns.push_back(ret); }
    virtual void removeReturn(Exp *e);
    virtual int getNumReturns();
    virtual Exp *getReturnExp(int n);
    void         setReturnExp(int n, Exp* e);
    virtual Type *getReturnType(int n);
    virtual void setReturnType(int n, Type *ty);
    virtual int findReturn(Exp *e);
    void fixReturnsWithParameters();

    // get/set the name
    virtual const char *getName();
    virtual void setName(const char *nam);

    // add a new parameter to this signature
    virtual void addParameter(const char *nam = NULL);
    virtual void addParameter(Type *type, const char *nam = NULL, 
                              Exp *e = NULL);
    virtual void addParameter(Exp *e);
    virtual void addParameter(Parameter *param);
    virtual void addEllipsis() { ellipsis = true; }
    virtual void removeParameter(Exp *e);
    virtual void removeParameter(int i);
    // set the number of parameters using defaults
    virtual void setNumParams(int n);

    // accessors for parameters
    virtual int getNumParams();
    virtual const char *getParamName(int n);
    virtual Exp *getParamExp(int n);
    virtual Type *getParamType(int n);
    virtual void setParamType(int n, Type *ty);
    virtual int findParam(Exp *e);
    virtual int findParam(const char *nam);
    // accessor for argument expressions
    virtual Exp *getArgumentExp(int n);
    virtual bool hasEllipsis() { return ellipsis; }
    std::list<Exp*> *getCallerSave(Prog* prog);

    // add a new implicit parameter
    virtual void addImplicitParameter(Exp *e);
    virtual void removeImplicitParameter(int i);

    // accessors for implicit params
    virtual int getNumImplicitParams();
    virtual const char *getImplicitParamName(int n);
    virtual Exp *getImplicitParamExp(int n);
    virtual Type *getImplicitParamType(int n);
    virtual int findImplicitParam(Exp *e);

    // analysis determines parameters / return type
    //virtual void analyse(UserProc *p);

    // any signature can be promoted to a higher level signature, if available
    virtual Signature *promote(UserProc *p);
    void print(std::ostream &out);
    void printToLog();

    virtual void getInternalStatements(StatementList &stmts);

    // Special for Mike: find the location that conventionally holds
    // the first outgoing (actual) parameter
    // MVE: Use the below now
    Exp* getFirstArgLoc(Prog* prog);

    // This is like getParamLoc, except that it works before Signature::analyse
    // is called.
    // It is used only to order parameters correctly, for the common case
    // where the proc will end up using a standard calling convention
    Exp* getEarlyParamExp(int n, Prog* prog);

    // Get a wildcard to find stack locations
    virtual Exp *getStackWildcard() { return NULL; }
    virtual int  getStackRegister(          ) {return 0; };
            int  getStackRegister(Prog* prog);
            bool isStackLocal(Prog* prog, Exp *e);

    // Quick and dirty hack
static Exp* getReturnExp2(BinaryFile* pBF);
static StatementList& getStdRetStmt(Prog* prog);

    // get anything that can be proven as a result of the signature
    virtual Exp *getProven(Exp *left) { return NULL; }

    virtual bool isPromoted() { return false; }

    // ascii versions of platform, calling convention name
static char*   platformName(platform plat);
static char*   conventionName(callconv cc);
};

class CustomSignature : public Signature {
protected:
    int sp;
public:
    CustomSignature(const char *nam);
    virtual ~CustomSignature() { }
    virtual bool isPromoted() { return true; }
    virtual Signature *clone();
    void setSP(int nsp);
    virtual int  getStackRegister(          ) {return sp; };
};

#endif
