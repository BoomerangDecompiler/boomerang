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

class Statement;

// Used to represent local variables (registers, stack locations, etc)
class Local {
protected:
	std::string name;
	Type* type;
	Exp *loc;

public:
	Local(const char *nam, Type *t, Exp *e) : name(nam), type(t), loc(e) { }
};

// Used to represent global variables
class Global {
protected:
	std::string name;
	Type* type;
	ADDRESS addr;

public:
	Global(const char *nam, Type *t, ADDRESS a) : name(nam), type(t), addr(a) { }
};

class TypedExp;

class Parameter { 
private:
    Type *type;
    std::string name;
    bool out;

public: 
    Parameter(Type *type, const char *name, bool out = false) : type(type),
   	 name(name), out(out) { }

    Type *getType() { return type; }
    const char *getName() { return name.c_str(); }
    bool isOut() { return out; }
};

class Signature {
protected:
    std::string name;						// name of procedure
    std::vector<Parameter*> params;

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
    virtual Type *getReturnType();
    virtual void setReturnType(Type *t);

    // get/set the name
    virtual const char *getName();
    virtual void setName(const char *nam);

    // add a new parameter to this signature
    virtual void addParameter(const char *nam = NULL);
    virtual void addParameter(Type *type, const char *nam = NULL);
    virtual void addParameter(Parameter *param) { params.push_back(param); }
    // set the number of parameters using defaults
    virtual void setNumParams(unsigned int n);

    // accessors for parameters
    virtual unsigned int getNumParams();
    virtual const char *getParamName(unsigned int n);
    virtual Exp *getParamExp(unsigned int n);
    virtual Type *getParamType(unsigned int n);
    // accessor for argument expressions
    virtual Exp *getArgumentExp(unsigned int n);

    // analysis determines parameters / return type
    virtual void analyse(UserProc *p);

    // any signature can be promoted to a higher level signature, if available
    virtual Signature *promote(UserProc *p);
    void print(std::ostream &out);

    virtual void getInternalStatements(std::list<Statement*> &stmts);
};

#endif
