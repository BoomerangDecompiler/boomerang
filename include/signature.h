/*
 * Copyright (C) 2002, Trent Waddington
 */
/*==============================================================================
 * FILE:	   signature.h
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
#include "memo.h"

class Statement;
class BinaryFile;
class XMLProgParser;

class Parameter : public Memoisable { 
private:
	Type *type;
	std::string name;
	Exp *exp;

public: 
			Parameter(Type *type, const char *name, Exp *exp = NULL) :
			  type(type), name(name), exp(exp)	{ }
			~Parameter() { delete type; delete exp; }
	bool	operator==(Parameter& other);
	Parameter* clone();

	Type *getType() { return type; }
	void setType(Type *ty) { type = ty; }
	const char *getName() { return name.c_str(); }
	void setName(const char *nam) { name = nam; }
	Exp *getExp()		{ return exp; }
	void setExp(Exp *e) { exp = e; }

	virtual Memo *makeMemo(int mId);
	virtual void readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
	Parameter() : type(NULL), name(""), exp(NULL) { }
};

class ImplicitParameter : public Parameter {
private:
	Parameter *parent;

public:
	ImplicitParameter(Type *type, const char *name, Exp *exp, Parameter *parent) : 
							Parameter(type, name, exp), parent(parent) { }
	~ImplicitParameter() { }
	ImplicitParameter* clone();

	void setParent(Parameter *p) { parent = p; }
	Parameter *getParent() { return parent; }

	virtual Memo *makeMemo(int mId);
	virtual void readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
	ImplicitParameter() : Parameter(), parent(NULL) { }
};

class Return : public Memoisable {
private:
		Type		*type;
		Exp			*exp;

public:
					Return(Type *type, Exp *exp) : type(type), exp(exp) { }
					~Return() { delete type; delete exp; }
		bool		operator==(Return& other);
		Return*		clone();

		Type 		*getType() { return type; }
		void		setType(Type *ty) { type = ty; }
		Exp			*getExp() { return exp; }
		Exp*&		getRefExp() {return exp;}
		void		setExp(Exp* e) { exp = e; }

virtual Memo		*makeMemo(int mId);
virtual void		readMemo(Memo *m, bool dec);

protected:
					Return() : type(NULL), exp(NULL) { }
		friend class XMLProgParser;
};		// class Return

class Signature : public Memoisable {
protected:
		std::string	name;		// name of procedure
		std::vector<Parameter*> params;
		std::vector<ImplicitParameter*> implicitParams;
		std::vector<Return*> returns;
		Type		*rettype;
		bool		ellipsis;
		Type		*preferedReturn;
		std::string	preferedName;
		std::vector<int> preferedParams;
		bool		unknown;
		bool		bFullSig;			// True if have a full signature from a signature file etc

		void		updateParams(UserProc *p, Statement *stmt, bool checkreach = true);
		bool		usesNewParam(UserProc *p, Statement *stmt, bool checkreach, int &n);

		void		addImplicitParametersFor(Parameter *p);
		void		addImplicitParameter(Type *type, const char *name, Exp *e, Parameter *parent);

public:
					Signature(const char *nam);
		// Platform plat, calling convention cc (both enums)
		// nam is name of the procedure (no longer stored in the Proc)
static	Signature	*instantiate(platform plat, callconv cc, const char *nam);
virtual				~Signature() { }

virtual bool		operator==(Signature& other);

		// clone this signature
virtual	Signature	*clone();

		bool		isUnknown() { return unknown; }
		void		setUnknown(bool b) { unknown = b; }
		void		setFullSig(bool full) {bFullSig = full;}

		// get the return location
virtual void		addReturn(Type *type, Exp *e = NULL);
virtual void		addReturn(Exp *e);
virtual void		addReturn(Return *ret) { returns.push_back(ret); }
virtual void		removeReturn(Exp *e);
virtual int			getNumReturns();
virtual Exp			*getReturnExp(int n);
		void		setReturnExp(int n, Exp* e);
virtual Type		*getReturnType(int n);
virtual void		setReturnType(int n, Type *ty);
virtual int			findReturn(Exp *e);
		void		fixReturnsWithParameters();
		void		setRetType(Type *t) { rettype = t; }

		// get/set the name
virtual const char	*getName();
virtual void		setName(const char *nam);

		// add a new parameter to this signature
virtual void		addParameter(const char *nam = NULL);
virtual void		addParameter(Type *type, const char *nam = NULL, Exp *e = NULL);
virtual void		addParameter(Exp *e);
virtual void		addParameter(Parameter *param);
		void		addEllipsis() { ellipsis = true; }
		void		killEllipsis() {ellipsis = false; }
virtual void		removeParameter(Exp *e);
virtual void		removeParameter(int i);
		// set the number of parameters using defaults
virtual void		setNumParams(int n);

		// accessors for parameters
virtual int			getNumParams();
virtual const char	*getParamName(int n);
virtual Exp			*getParamExp(int n);
virtual Type		*getParamType(int n);
virtual void		setParamType(int n, Type *ty);
virtual void		setParamName(int n, const char *name);
virtual void		setParamExp(int n, Exp *e);
virtual int			findParam(Exp *e);
virtual int			findParam(const char *nam);
		// accessor for argument expressions
virtual Exp			*getArgumentExp(int n);
virtual bool		hasEllipsis() { return ellipsis; }
		std::list<Exp*> *getCallerSave(Prog* prog);

		void		renameParam(const char *oldName, const char *newName);

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

		// Data flow based type analysis. Meet the parameters with their current types
		// Returns true if a change
		bool		dfaTypeAnalysis(Cfg* cfg);

		// any signature can be promoted to a higher level signature, if available
virtual Signature *promote(UserProc *p);
	void print(std::ostream &out);
	void printToLog();

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
virtual Exp			*getStackWildcard() { return NULL; }
virtual int			getStackRegister(			) {
						//assert(0);
						return 28; };
		int			getStackRegister(Prog* prog);
		// Does expression e represent a local stack-based variable?
		// Result can be ABI specific, e.g. sparc has locals in the parent's stack frame,
		// at POSITIVE offsets from the stack pointer register
		// Also, I believe that the PA/RISC stack grows away from 0
		bool		isStackLocal(Prog* prog, Exp *e);
		// Similar to the above, but checks for address of a local (i.e. sp{0} -/+ K)
virtual	bool		isAddrOfStackLocal(Prog* prog, Exp* e);
		// For most machines, local variables are always NEGATIVE offsets from sp
virtual bool		isLocalOffsetNegative() {return true;}
		// For most machines, local variables are not POSITIVE offsets from sp
virtual bool		isLocalOffsetPositive() {return false;}
		// Is this operator (between the stack pointer and a constant) compatible with a stack local pattern?
		bool		isOpCompatStackLocal(OPER op);	

		// Quick and dirty hack
static	Exp*		getReturnExp2(BinaryFile* pBF);
static	StatementList& getStdRetStmt(Prog* prog);

		// get anything that can be proven as a result of the signature
virtual Exp			*getProven(Exp *left) { return NULL; }

		// Return true if this is a known machine (e.g. SparcSignature as opposed to Signature)
virtual bool		isPromoted() { return false; }
		// Return true if this has a full blown signature, e.g. main/WinMain etc.
		// Note that many calls to isFullSignature were incorrectly calls to isPromoted()
		bool		isFullSignature() {return bFullSig;}

	// ascii versions of platform, calling convention name
static char*		platformName(platform plat);
static char*		conventionName(callconv cc);
virtual platform	getPlatform() { return PLAT_GENERIC; }
virtual callconv	getConvention() { return CONV_NONE; }

	// prefered format
	void setPreferedReturn(Type *ty) { preferedReturn = ty; }
	void setPreferedName(const char *nam) { preferedName = nam; }
	void addPreferedParameter(int n) { preferedParams.push_back(n); }
	Type *getPreferedReturn() { return preferedReturn; }
	const char *getPreferedName() { return preferedName.c_str(); }
	unsigned int getNumPreferedParams() { return preferedParams.size(); }
	int getPreferedParam(int n) { return preferedParams[n]; }

	virtual Memo *makeMemo(int mId);
	virtual void readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
	Signature() : name(""), rettype(NULL), ellipsis(false), preferedReturn(NULL), preferedName("") { }
	void appendParameter(Parameter *p) { params.push_back(p); }
	void appendImplicitParameter(ImplicitParameter *p) { implicitParams.push_back(p); }
	void appendReturn(Return *r) { returns.push_back(r); }
};	// class Signature

class CustomSignature : public Signature {
protected:
	int sp;
public:
	CustomSignature(const char *nam);
	virtual ~CustomSignature() { }
	virtual bool isPromoted() { return true; }
	virtual Signature *clone();
	void setSP(int nsp);
	virtual int	 getStackRegister(			) {return sp; };
};

#endif
