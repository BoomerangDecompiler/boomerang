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
#include "exp.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "signature.h"
#include "util.h"
#include "cfg.h"
#include "dataflow.h"
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
		virtual Type *getReturnType();
		virtual void setReturnType(Type *t);

		virtual	void addParameter(const char *nam = NULL);
		virtual void addParameter(Type *type, const char *nam = NULL);
		virtual void setNumParams(unsigned int n);

		virtual unsigned int getNumParams();
		virtual	Exp *getParamExp(unsigned int n);
		virtual PARAM_DIR getParamDirection(unsigned int n);
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
			virtual Type *getReturnType();
			virtual void setReturnType(Type *t);

			virtual	void addParameter(const char *nam = NULL);
			virtual void addParameter(Type *type, const char *nam = NULL);
			virtual void setNumParams(unsigned int n);

			virtual unsigned int getNumParams();
			virtual	Exp *getParamExp(unsigned int n);
			virtual PARAM_DIR getParamDirection(unsigned int n);
			virtual Exp *getArgumentExp(unsigned int n);

			virtual void analyse(UserProc *p);

			virtual Signature *promote(UserProc *p);
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
			virtual Type *getReturnType();
			virtual void setReturnType(Type *t);

			virtual	void addParameter(const char *nam = NULL);
			virtual void addParameter(Type *type, const char *nam = NULL);
			virtual void setNumParams(unsigned int n);

			virtual unsigned int getNumParams();
			virtual	Exp *getParamExp(unsigned int n);
			virtual PARAM_DIR getParamDirection(unsigned int n);
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
	n->pnames = pnames;
	n->ptypes = ptypes;
	n->inregs = inregs;
	n->outregs = outregs;
	return n;
}

bool CallingConvention::Win32Signature::operator==(const Signature& other) const
{
	// TODO
	return false;
}

bool CallingConvention::Win32Signature::qualified(UserProc *p, Signature &candidate)
{
	std::vector<int> &inregs = candidate.getInRegs();
	std::vector<int> &outregs = candidate.getOutRegs();
	// must be callee pop
	if (p->getBytesPopped() == 0) return false;

	// better be win32
	if (prog.pBF->GetFormat() != LOADFMT_PE) return false;

	// better be x86
	if (std::string(prog.pFE->getFrontEndId()) != "pentium") return false;
	
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

	return true;
}

bool CallingConvention::Win32Signature::serialize(std::ostream &ouf, int len)
{
	std::streampos st = ouf.tellp();

	char type = 0;
	saveValue(ouf, type, false);
	saveString(ouf, name);

	for (unsigned int i = 0; i < pnames.size(); i++) {
		saveFID(ouf, FID_SIGNATURE_PARAM);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		saveString(ouf, pnames[i]);
		int l;
		ptypes[i]->serialize(ouf, l);

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
				pnames.push_back(nam);
				ptypes.push_back(t);
				delete t;
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

Type *CallingConvention::Win32Signature::getReturnType()
{	
	return rettype;
}

void CallingConvention::Win32Signature::setReturnType(Type *t)
{	
	if (rettype) delete rettype;
	rettype = t;
}

void CallingConvention::Win32Signature::addParameter(const char *nam /*= NULL*/)
{
	if (nam == NULL) {
		std::stringstream os;
		os << "arg" << pnames.size() << std::ends;
		std::string s = os.str();
		addParameter(s.c_str());
	}
	else
		addParameter(nam);
}

void CallingConvention::Win32Signature::addParameter(Type *type, const char *nam /*= NULL*/)
{
	ptypes.push_back(type);
	if (nam == NULL) {
		std::stringstream os;
		os << "arg" << pnames.size()+1 << std::ends;
		std::string s = os.str();
		pnames.push_back(s.c_str());
	}
	else
		pnames.push_back(nam);
}

void CallingConvention::Win32Signature::setNumParams(unsigned int n)
{
	if (n < pnames.size()) {
		// truncate
		pnames.erase(pnames.begin() + n, pnames.end());
		ptypes.erase(ptypes.begin() + n, ptypes.end());
	} else {
		for (unsigned int i = pnames.size(); i < n; i++)
			addParameter();		
	}
}

unsigned int CallingConvention::Win32Signature::getNumParams()
{
	return pnames.size();
}

Exp *CallingConvention::Win32Signature::getParamExp(unsigned int n)
{
	assert(n < pnames.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

PARAM_DIR CallingConvention::Win32Signature::getParamDirection(unsigned int n)
{
	return D_IN;
}

Exp *CallingConvention::Win32Signature::getArgumentExp(unsigned int n)
{
	assert(n < pnames.size());
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
	n->pnames = pnames;
	n->ptypes = ptypes;
	n->inregs = inregs;
	n->outregs = outregs;
	return n;
}

bool CallingConvention::StdC::PentiumSignature::operator==(const Signature& other) const
{
	// TODO
	return false;
}


bool CallingConvention::StdC::PentiumSignature::qualified(UserProc *p, Signature &candidate)
{
	std::vector<int> &inregs = candidate.getInRegs();
	std::vector<int> &outregs = candidate.getOutRegs();

	// better be x86
	if (std::string(prog.pFE->getFrontEndId()) != "pentium") return false;
	
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

	return true;
}

bool CallingConvention::StdC::PentiumSignature::serialize(std::ostream &ouf, int len)
{
	std::streampos st = ouf.tellp();

	char type = 0;
	saveValue(ouf, type, false);
	saveString(ouf, name);

	for (unsigned int i = 0; i < pnames.size(); i++) {
		saveFID(ouf, FID_SIGNATURE_PARAM);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		saveString(ouf, pnames[i]);
		int l;
		ptypes[i]->serialize(ouf, l);

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
				pnames.push_back(nam);
				ptypes.push_back(t);
				delete t;
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

Type *CallingConvention::StdC::PentiumSignature::getReturnType()
{
	return rettype;
}

void CallingConvention::StdC::PentiumSignature::setReturnType(Type *t)
{
	if (rettype) delete rettype;
	rettype = t;
}

void CallingConvention::StdC::PentiumSignature::addParameter(const char *nam /*= NULL*/)
{
	addParameter(new IntegerType(), nam);
}

void CallingConvention::StdC::PentiumSignature::addParameter(Type *type, const char *nam /*= NULL*/)
{
	ptypes.push_back(type);
	if (nam)
		pnames.push_back(nam);
	else {
		std::stringstream os;
		os << "arg" << pnames.size() + 1 << std::ends;
		std::string s = os.str();
		pnames.push_back(s.c_str());
	}
}

void CallingConvention::StdC::PentiumSignature::setNumParams(unsigned int n)
{
	if (n < pnames.size()) {
		// truncate
		pnames.erase(pnames.begin() + n, pnames.end());
		ptypes.erase(ptypes.begin() + n, ptypes.end());
	} else {
		for (unsigned int i = pnames.size(); i < n; i++) {
			addParameter();
		}
	}
}

unsigned int CallingConvention::StdC::PentiumSignature::getNumParams()
{
	return pnames.size();
}

Exp *CallingConvention::StdC::PentiumSignature::getParamExp(unsigned int n)
{
	assert(n < pnames.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

PARAM_DIR CallingConvention::StdC::PentiumSignature::getParamDirection(unsigned int n)
{
	return D_IN;
}

Exp *CallingConvention::StdC::PentiumSignature::getArgumentExp(unsigned int n)
{
	assert(n < pnames.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const(n * 4)));
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

CallingConvention::StdC::SparcSignature::SparcSignature(const char *nam) : Signature(nam), rettype(NULL)
{

}

CallingConvention::StdC::SparcSignature::SparcSignature(Signature &old) : Signature(old)
{

}

Signature *CallingConvention::StdC::SparcSignature::clone()
{
	SparcSignature *n = new SparcSignature(name.c_str());
	n->pnames = pnames;
	n->ptypes = ptypes;
	n->inregs = inregs;
	n->outregs = outregs;
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

	for (unsigned int i = 0; i < pnames.size(); i++) {
		saveFID(ouf, FID_SIGNATURE_PARAM);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		saveString(ouf, pnames[i]);
		int l;
		ptypes[i]->serialize(ouf, l);

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
				pnames.push_back(nam);
				ptypes.push_back(t);
				delete t;
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

Type *CallingConvention::StdC::SparcSignature::getReturnType()
{
	return rettype;
}

void CallingConvention::StdC::SparcSignature::setReturnType(Type *t)
{
	if (rettype) delete rettype;
	rettype = t;
}

void CallingConvention::StdC::SparcSignature::addParameter(const char *nam /*= NULL*/)
{
	addParameter(new IntegerType(), nam);
}

void CallingConvention::StdC::SparcSignature::addParameter(Type *type, const char *nam /*= NULL*/)
{
	ptypes.push_back(type);
	if (nam)
		pnames.push_back(nam);
	else {
		std::stringstream os;
		os << "arg" << pnames.size() + 1 << std::ends;
		std::string s = os.str();
		pnames.push_back(s.c_str());
	}
}

void CallingConvention::StdC::SparcSignature::setNumParams(unsigned int n)
{
	if (n < pnames.size()) {
		// truncate
		pnames.erase(pnames.begin() + n, pnames.end());
		ptypes.erase(ptypes.begin() + n, ptypes.end());
	} else {
		for (unsigned int i = pnames.size(); i < n; i++) {
			addParameter();
		}
	}
}

unsigned int CallingConvention::StdC::SparcSignature::getNumParams()
{
	return pnames.size();
}

Exp *CallingConvention::StdC::SparcSignature::getParamExp(unsigned int n)
{
	assert(n < pnames.size());
	Exp *esp = new Unary(opRegOf, new Const(28));
	return new Unary(opMemOf, new Binary(opPlus, esp, new Const((n+1) * 4)));
}

PARAM_DIR CallingConvention::StdC::SparcSignature::getParamDirection(unsigned int n)
{
	return D_IN;
}

Exp *CallingConvention::StdC::SparcSignature::getArgumentExp(unsigned int n)
{
	assert(n < pnames.size());
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

Signature::Signature(const char *nam)
{
	name = nam;
}

Signature *Signature::clone()
{
	Signature *n = new Signature(name.c_str());
	n->pnames = pnames;
	n->ptypes = ptypes;
	n->inregs = inregs;
	n->outregs = outregs;
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

	for (unsigned int i = 0; i < pnames.size(); i++) {
		saveFID(ouf, FID_SIGNATURE_PARAM);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		saveString(ouf, pnames[i]);
		int l;
		ptypes[i]->serialize(ouf, l);
		saveValue(ouf, inregs[i], false);

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
				pnames.push_back(nam);
				ptypes.push_back(t);
				int r;
				loadValue(inf, r, false);
				inregs.push_back(r);
				delete t;
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
	static VoidType t;
	return &t;
}

void Signature::setReturnType(Type *t)
{
	assert(false);
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
	assert(false);
}

void Signature::addParameter(Type *type, const char *nam /*= NULL*/)
{
	assert(false);
}

void Signature::setNumParams(unsigned int n)
{
	assert(false);
}

unsigned int Signature::getNumParams()
{
	return inregs.size() + outregs.size();
}

const char *Signature::getParamName(unsigned int n)
{
	assert(n < pnames.size());
	return pnames[n].c_str();
}

Exp *Signature::getParamExp(unsigned int n)
{
	if (n < inregs.size()) {
		return new Unary(opRegOf, new Const(inregs[n]));
	}
	n -= inregs.size();
	assert(n < outregs.size());
	return new Unary(opRegOf, new Const(outregs[n]));	
}

Type *Signature::getParamType(unsigned int n)
{
	assert(n < ptypes.size());
	return ptypes[n];
}

PARAM_DIR Signature::getParamDirection(unsigned int n)
{
	if (n < inregs.size())
		return D_IN;
	assert(n < inregs.size() + outregs.size());
	return D_OUT;
}

Exp *Signature::getArgumentExp(unsigned int n)
{
	// TODO: esp?
	return getParamExp(n);
}

void Signature::findInRegs(UserProc *p)
{
	UseSet uses;
	p->getCFG()->getAllUses(uses);
	std::set<int> set_inregs;
	for (UseSet::iterator it = uses.begin(); it != uses.end(); it++) {
		Exp *e = (*it).getExp();
		if (e->getOper() == opSubscript) {
			Const *c = (Const*)e->getSubExp2();
			assert(c->getOper() == opIntConst);
			if (c->getInt() == 0) {
				if (e->getSubExp1()->getOper() == opRegOf) {
					Const *r = (Const*)e->getSubExp1()->getSubExp1();
					if (r->getOper() == opIntConst) {
						set_inregs.insert(r->getInt());
					}
				}
			}
		}
	}
	inregs.clear();
	for (std::set<int>::iterator sit = set_inregs.begin(); sit != set_inregs.end(); sit++)
		inregs.push_back(*sit);
}

void Signature::findOutRegs(UserProc *p)
{
	DefSet defs;
	p->getCFG()->getSSADefs(defs);
	std::set<int> set_outregs;
	for (DefSet::iterator it = defs.begin(); it != defs.end(); it++) {
		Exp *e = (*it).getLeft();
		if (e->getOper() == opSubscript) {
			if (e->getSubExp1()->getOper() == opRegOf) {
				Const *r = (Const*)e->getSubExp1()->getSubExp1();
				if (r->getOper() == opIntConst) {
					set_outregs.insert(r->getInt());
				}
			}
		}	
	}
	outregs.clear();
	for (std::set<int>::iterator sit = set_outregs.begin(); sit != set_outregs.end(); sit++)
		outregs.push_back(*sit);
}

void Signature::analyse(UserProc *p)
{
	p->transformToSSAForm();
	findInRegs(p);
	findOutRegs(p);
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

Signature *Signature::DefaultLibrarySignature(const char *nam)
{
	if (prog.pBF->GetFormat() == LOADFMT_PE) {
		return new CallingConvention::Win32Signature(nam);
	}
	std::string fe = prog.pFE->getFrontEndId();
	if (fe == "pentium") {
		return new CallingConvention::StdC::PentiumSignature(nam);
	}
	if (fe == "sparc") {
		return new CallingConvention::StdC::SparcSignature(nam);
	}
	// insert other platforms here
	assert(false);
	return NULL;	
}

Signature *Signature::instantiate(const char *str, const char *nam)
{
	std::string s = str;
	if (s == "-win32") {
		assert(prog.pBF->GetFormat() == LOADFMT_PE);
		return new CallingConvention::Win32Signature(nam);
	}
	if (s == "-stdc") {
		std::string fe = prog.pFE->getFrontEndId();
		if (fe == "pentium") {
			return new CallingConvention::StdC::PentiumSignature(nam);
		}
		if (fe == "sparc") {
			return new CallingConvention::StdC::SparcSignature(nam);
		}
		// insert other platforms here
		assert(false);
	}
	// insert other conventions here
	assert(false);
	return NULL;
}
