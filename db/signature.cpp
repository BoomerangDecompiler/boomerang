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
 * 18 Jul 02 = Mike: Changed addParameter's last param to deflt to "", not NULL
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

namespace CallingConvention {

	class Win32Signature : public Signature {
	protected:
		Type *rettype;

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

		virtual	Exp *getParamExp(unsigned int n);
		virtual Exp *getArgumentExp(unsigned int n);

		virtual void analyse(UserProc *p);

		virtual Signature *promote(UserProc *p);
	};

	namespace StdC {
		class PentiumSignature : public Signature {
		protected:
			Type *rettype;

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

			virtual	Exp *getParamExp(unsigned int n);
			virtual Exp *getArgumentExp(unsigned int n);

			virtual void analyse(UserProc *p);

			virtual Signature *promote(UserProc *p);
                        virtual void getInternalStatements(std::list<Statement*> &stmts);
		};		

		class SparcSignature : public Signature {
		protected:
			Type *rettype;

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

			virtual	Exp *getParamExp(unsigned int n);
			virtual Exp *getArgumentExp(unsigned int n);

			virtual void analyse(UserProc *p);

			virtual Signature *promote(UserProc *p);
		};
	};
};

CallingConvention::Win32Signature::Win32Signature(const char *nam) : Signature(nam), rettype(NULL)
{

}

CallingConvention::Win32Signature::Win32Signature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::Win32Signature::clone()
{
	Win32Signature *n = new Win32Signature(name.c_str());
	n->params = params;
	return n;
}

bool CallingConvention::Win32Signature::operator==(const Signature& other) const
{
	// TODO
	return false;
}

bool CallingConvention::Win32Signature::qualified(UserProc *p, Signature &candidate)
{
/*
	std::vector<int> &inregs = candidate.getInRegs();
	std::vector<int> &outregs = candidate.getOutRegs();
	// must be callee pop
	if (p->getBytesPopped() == 0) return false;

	// better be win32 (disabled)
	//if (prog->pBF->GetFormat() != LOADFMT_PE) return false;

	// better be x86 (disabled)
	//if (std::string(prog->pFE->getFrontEndId()) != "pentium") return false;
	
	// debug
	std::stringstream os;
	for (unsigned int i = 0; i < inregs.size(); i++) {
		os << inregs[i] << ", ";
	}
	std::string s = os.str();

	// esp must be the only inreg
	if (inregs.size() != 1 || inregs[0] != 28) return false;

	// eax must be the only outreg (if any)
	if (outregs.size() == 1 && outregs[0] != 24) return false;
	if (outregs.size() > 1) return false;
*/
	return true;
}

bool CallingConvention::Win32Signature::serialize(std::ostream &ouf, int len)
{
	std::streampos st = ouf.tellp();

	char type = 0;
	saveValue(ouf, type, false);
	saveString(ouf, name);

	for (unsigned int i = 0; i < params.size(); i++) {
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

Exp *CallingConvention::Win32Signature::getParamExp(unsigned int n)
{
	assert(n < params.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

Exp *CallingConvention::Win32Signature::getArgumentExp(unsigned int n)
{
	assert(n < params.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const(n * 4)));
}

void CallingConvention::Win32Signature::analyse(UserProc *p)
{
	assert((p->getBytesPopped() % 4) == 0);
	setNumParams(p->getBytesPopped() / 4);
	// TODO: update callers
}

Signature *CallingConvention::Win32Signature::promote(UserProc *p)
{
	// no promotions from win32 signature up, yet.
	// a possible thing to investigate would be COM objects
	return this;
}

CallingConvention::StdC::PentiumSignature::PentiumSignature(const char *nam) : Signature(nam), rettype(NULL)
{

}

CallingConvention::StdC::PentiumSignature::PentiumSignature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::StdC::PentiumSignature::clone()
{
	PentiumSignature *n = new PentiumSignature(name.c_str());
	n->params = params;
	return n;
}

bool CallingConvention::StdC::PentiumSignature::operator==(const Signature& other) const
{
	// TODO
	return false;
}


bool CallingConvention::StdC::PentiumSignature::qualified(UserProc *p, Signature &candidate)
{
	/*
	std::vector<int> &inregs = candidate.getInRegs();
	std::vector<int> &outregs = candidate.getOutRegs();

	// better be x86 (disabled)
	//if (std::string(prog->pFE->getFrontEndId()) != "pentium") return false;
	
	// debug
	std::stringstream os;
	for (unsigned int i = 0; i < inregs.size(); i++) {
		os << inregs[i] << ", ";
	}
	std::string s = os.str();

	// esp must be the only inreg
	if (inregs.size() != 1 || inregs[0] != 28) return false;

	// eax must be the only outreg (if any)
	if (outregs.size() == 1 && outregs[0] != 24) return false;
	if (outregs.size() > 1) return false;
	*/

	return true;
}

bool CallingConvention::StdC::PentiumSignature::serialize(std::ostream &ouf, int len)
{
	std::streampos st = ouf.tellp();

	char type = 0;
	saveValue(ouf, type, false);
	saveString(ouf, name);

	for (unsigned int i = 0; i < params.size(); i++) {
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

Exp *CallingConvention::StdC::PentiumSignature::getParamExp(unsigned int n)
{
	assert(n < params.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

Exp *CallingConvention::StdC::PentiumSignature::getArgumentExp(unsigned int n)
{
	assert(n < params.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
        //if (n == 0)
	//    return new Unary(opMemOf, esp);
	return new Unary(opMemOf, new Binary(opPlus, esp, 
				new Const((int)((n+1) * 4))));
}

void CallingConvention::StdC::PentiumSignature::analyse(UserProc *p)
{
	// TODO
	assert(false);
}

Signature *CallingConvention::StdC::PentiumSignature::promote(UserProc *p)
{
	// No promotions from here up, obvious idea would be c++ name mangling	
	return this;
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

CallingConvention::StdC::SparcSignature::SparcSignature(const char *nam) : Signature(nam), rettype(NULL)
{

}

CallingConvention::StdC::SparcSignature::SparcSignature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::StdC::SparcSignature::clone()
{
	SparcSignature *n = new SparcSignature(name.c_str());
	n->params = params;
	return n;
}

bool CallingConvention::StdC::SparcSignature::operator==(const Signature& other) const
{
	// TODO
	return false;
}

bool CallingConvention::StdC::SparcSignature::qualified(UserProc *p, Signature &candidate)
{
	// TODO
	return false;
}

bool CallingConvention::StdC::SparcSignature::serialize(std::ostream &ouf, int len)
{
	std::streampos st = ouf.tellp();

	char type = 0;
	saveValue(ouf, type, false);
	saveString(ouf, name);

	for (unsigned int i = 0; i < params.size(); i++) {
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
	// ment will be to %i0 (register 24).
	return new Unary(opRegOf, new Const(8));
}

Exp *CallingConvention::StdC::SparcSignature::getParamExp(unsigned int n)
{
	assert(n < params.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

Exp *CallingConvention::StdC::SparcSignature::getArgumentExp(unsigned int n)
{
	assert(n < params.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const(n * 4)));
}

void CallingConvention::StdC::SparcSignature::analyse(UserProc *p)
{
	// TODO
	assert(false);
}

Signature *CallingConvention::StdC::SparcSignature::promote(UserProc *p)
{
	// no promotions from here up, obvious example would be name mangling
	return this;
}

Signature::Signature(const char *nam) : rettype(NULL)
{
	name = nam;
}

Signature *Signature::clone()
{
	Signature *n = new Signature(name.c_str());
	n->params = params;
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

	for (unsigned int i = 0; i < params.size(); i++) {
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

void Signature::setNumParams(unsigned int n)
{
	if (n < params.size()) {
		// truncate
		params.erase(params.begin() + n, params.end());
	} else {
		for (unsigned int i = params.size(); i < n; i++)
			addParameter();		
	}
}

unsigned int Signature::getNumParams()
{
	return params.size();
}

const char *Signature::getParamName(unsigned int n)
{
	assert(n < params.size());
	return params[n]->getName();
}

Exp *Signature::getParamExp(unsigned int n)
{
	assert(false);
}

Type *Signature::getParamType(unsigned int n)
{
	assert(n < params.size());
	return params[n]->getType();
}

Exp *Signature::getArgumentExp(unsigned int n)
{
	// TODO: esp?
	return getParamExp(n);
}

void Signature::analyse(UserProc *p) {
	// TODO
}

Signature *Signature::promote(UserProc *p)
{
	analyse(p);

	if (CallingConvention::Win32Signature::qualified(p, *this)) {
		Signature *sig = new CallingConvention::Win32Signature(*this);
		delete this;
		sig->analyse(p);
		return sig;
	}

	if (CallingConvention::StdC::PentiumSignature::qualified(p, *this)) {
		Signature *sig = new CallingConvention::StdC::PentiumSignature(*this);
		delete this;
		sig->analyse(p);
		return sig;
	}

	if (CallingConvention::StdC::SparcSignature::qualified(p, *this)) {
		Signature *sig = new CallingConvention::StdC::SparcSignature(*this);
		delete this;
		sig->analyse(p);
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
    out << name << "(";
    for (int i = 0; i < params.size(); i++) {
        out << params[i]->getName();
        if (i != params.size()-1) out << ", ";
    }
    out << ")" << std::endl;
}

void Signature::getInternalStatements(std::list<Statement*> &stmts)
{
}

