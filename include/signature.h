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

class Statement;
class BinaryFile;

class Parameter { 
private:
    Type *type;
    std::string name;
    Exp *exp;

public: 
    Parameter(Type *type, const char *name, Exp *exp = NULL) : type(type),
   	 name(name), exp(exp) { }
    ~Parameter() { delete type; delete exp; }

    Type *getType() { return type; }
    const char *getName() { return name.c_str(); }
    Exp *getExp() { return exp; }
};

class Return {
private:
    Type *type;
    Exp *exp;

public:
    Return(Type *type, Exp *exp) : type(type), exp(exp) { }
    ~Return() { delete type; delete exp; }

    Type *getType() { return type; }
    Exp *getExp() { return exp; }
};

class Signature {
protected:
    std::string name;       // name of procedure
    std::vector<Parameter*> params;
    std::vector<Return*> returns;
    Type *rettype;
    bool ellipsis;

    void updateParams(UserProc *p, Statement *stmt, bool checkreach = true);
    bool usesNewParam(UserProc *p, Statement *stmt, bool checkreach, int &n);

public:
    Signature(const char *nam);
    static Signature *instantiate(const char *str, const char *nam);
    virtual ~Signature() { }

    virtual bool operator==(const Signature& other) const;

    // clone this signature
    virtual Signature *clone();

    // serialization
    virtual bool serialize(std::ostream &ouf, int len);
    static Signature *deserialize(std::istream &inf);
    virtual bool deserialize_fid(std::istream &inf, int fid);

    // get the return location
    virtual Exp *getReturnExp();
    static  Exp *getReturnExp2(BinaryFile *pBF);
    virtual Type *getReturnType();
    virtual void setReturnType(Type *t);
    virtual void addReturn(Type *type, Exp *e = NULL);
    virtual void addReturn(Exp *e);
    virtual void addReturn(Return *ret) { returns.push_back(ret); }
    virtual int getNumReturns();
    virtual Exp *getReturnExp(int n);
    virtual Type *getReturnType(int n);

    // get/set the name
    virtual const char *getName();
    virtual void setName(const char *nam);

    // add a new parameter to this signature
    virtual void addParameter(const char *nam = NULL);
    virtual void addParameter(Type *type, const char *nam = NULL, 
                              Exp *e = NULL);
    virtual void addParameter(Exp *e);
    virtual void addParameter(Parameter *param) { params.push_back(param); }
    virtual void addEllipsis() { ellipsis = true; }
    // set the number of parameters using defaults
    virtual void setNumParams(int n);

    // accessors for parameters
    virtual int getNumParams();
    virtual const char *getParamName(int n);
    virtual Exp *getParamExp(int n);
    virtual Type *getParamType(int n);
    virtual int findParam(Exp *e);
    // accessor for argument expressions
    virtual Exp *getArgumentExp(int n);
    virtual bool hasEllipsis() { return ellipsis; }
    std::list<Exp*> *getCallerSave(Prog* prog);

    // analysis determines parameters / return type
    virtual void analyse(UserProc *p);

    // any signature can be promoted to a higher level signature, if available
    virtual Signature *promote(UserProc *p);
    void print(std::ostream &out);

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

    // Quick and dirty hack
static StatementList& getStdRetStmt(Prog* prog);

    // get anything that can be proven as a result of the signature
    virtual Exp *getProven(Exp *left) { return NULL; }
};

#endif
