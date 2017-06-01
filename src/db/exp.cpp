/*
 * Copyright (C) 2002-2006 Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file       exp.cpp
 * \brief   Implementation of the Exp and related classes.
 ******************************************************************************/

#include <cassert>
#include <numeric>   // For accumulate
#include <algorithm> // For std::max()
#include <map>       // In decideType()
#include <sstream>   // Need gcc 3.0 or better
#include <cstring>
#include "include/types.h"
#include "include/statement.h"
#include "db/cfg.h"
#include "db/exp.h"
#include "include/register.h"
#include "include/rtl.h" // E.g. class ParamEntry in decideType()
#include "db/proc.h"
#include "include/signature.h"
#include "db/prog.h"
#include "operstrings.h" // Defines a large array of strings for the createDotFile etc. functions. Needs -I. to find it
#include "include/util.h"
#include "boom_base/log.h"
#include "include/visitor.h"
#include "boom_base/log.h"
#include <QRegularExpression>
#include <iomanip>          // For std::setw etc

extern char debug_buffer[]; ///< For prints functions
static int tlstrchr(const QString& str, char ch);

// Derived class constructors

Const::Const(uint32_t i)
	: Exp(opIntConst)
	, m_conscript(0)
	, m_type(VoidType::get())
{
	u.i = i;
}


Const::Const(int i)
	: Exp(opIntConst)
	, m_conscript(0)
	, m_type(VoidType::get())
{
	u.i = i;
}


Const::Const(QWord ll)
	: Exp(opLongConst)
	, m_conscript(0)
	, m_type(VoidType::get())
{
	u.ll = ll;
}


Const::Const(double d)
	: Exp(opFltConst)
	, m_conscript(0)
	, m_type(VoidType::get())
{
	u.d = d;
}


Const::Const(const QString& p)
	: Exp(opStrConst)
	, m_conscript(0)
	, m_type(VoidType::get())
{
	m_string = p;
}


Const::Const(Function *p)
	: Exp(opFuncConst)
	, m_conscript(0)
	, m_type(VoidType::get())
{
	u.pp = p;
}


Const::Const(ADDRESS a)
	: Exp(opIntConst)
	, m_conscript(0)
	, m_type(VoidType::get())
{
	assert(a.isSourceAddr());
	u.a = a;
}


// Copy constructor
Const::Const(const Const& o)
	: Exp(o.m_oper)
{
	u           = o.u;
	m_conscript = o.m_conscript;
	m_type      = o.m_type;
	m_string    = o.m_string;
}


Terminal::Terminal(OPER _op)
	: Exp(_op)
{
}


Terminal::Terminal(const Terminal& o)
	: Exp(o.m_oper)
{
}


Unary::Unary(OPER _op)
	: Exp(_op)                    /*,subExp1(nullptr)*/
{
	// pointer uninitialized to help out finding usages of null pointers ?
	assert(m_oper != opRegOf);
}


Unary::Unary(OPER _op, SharedExp _e)
	: Exp(_op)
	, subExp1(_e)
{
	assert(subExp1);
}


Unary::Unary(const Unary& o)
	: Exp(o.m_oper)
{
	subExp1 = o.subExp1->clone();
	assert(subExp1);
}


Binary::Binary(OPER _op)
	: Unary(_op)
{
	// Initialise the 2nd pointer. The first pointer is initialised in the Unary constructor
	// subExp2 = 0;
}


Binary::Binary(OPER _op, SharedExp _e1, SharedExp _e2)
	: Unary(_op, _e1)
	, subExp2(_e2)
{
	assert(subExp1 && subExp2);
}


Binary::Binary(const Binary& o)
	: Unary(m_oper)
{
	setSubExp1(subExp1->clone());
	subExp2 = o.subExp2->clone();
	assert(subExp1 && subExp2);
}


Ternary::Ternary(OPER _op)
	: Binary(_op)
{
	subExp3 = nullptr;
}


Ternary::Ternary(OPER _op, SharedExp _e1, SharedExp _e2, SharedExp _e3)
	: Binary(_op, _e1, _e2)
{
	subExp3 = _e3;
	assert(subExp1 && subExp2 && subExp3);
}


Ternary::Ternary(const Ternary& o)
	: Binary(o.m_oper)
{
	subExp1 = o.subExp1->clone();
	subExp2 = o.subExp2->clone();
	subExp3 = o.subExp3->clone();
	assert(subExp1 && subExp2 && subExp3);
}


TypedExp::TypedExp()
	: Unary(opTypedExp)
	, type(nullptr)
{
}


TypedExp::TypedExp(SharedExp e1)
	: Unary(opTypedExp, e1)
	, type(nullptr)
{
}


TypedExp::TypedExp(SharedType ty, SharedExp e1)
	: Unary(opTypedExp, e1)
	, type(ty)
{
}


TypedExp::TypedExp(TypedExp& o)
	: Unary(opTypedExp)
{
	subExp1 = o.subExp1->clone();
	type    = o.type->clone();
}


FlagDef::FlagDef(SharedExp params, SharedRTL _rtl)
	: Unary(opFlagDef, params)
	, rtl(_rtl)
{
}


RefExp::RefExp(SharedExp e, Instruction *d)
	: Unary(opSubscript, e)
	, m_def(d)
{
	assert(e);
}


std::shared_ptr<RefExp> RefExp::get(SharedExp e, Instruction *def)
{
	return std::make_shared<RefExp>(e, def);
}


TypeVal::TypeVal(SharedType ty)
	: Terminal(opTypeVal)
	, val(ty)
{
}


Location::Location(OPER _op, SharedExp exp, UserProc *_p)
	: Unary(_op, exp)
	, proc(_p)
{
	assert(m_oper == opRegOf || m_oper == opMemOf || m_oper == opLocal || m_oper == opGlobal || m_oper == opParam || m_oper == opTemp);

	if (_p == nullptr) {
		// eep.. this almost always causes problems
		SharedExp e = exp;

		if (e) {
			bool giveUp = false;

			while (this->proc == nullptr && !giveUp) {
				switch (e->getOper())
				{
				case opRegOf:
				case opMemOf:
				case opTemp:
				case opLocal:
				case opGlobal:
				case opParam:
					this->proc = std::static_pointer_cast<Location>(e)->getProc();
					giveUp     = true;
					break;

				case opSubscript:
					e = e->getSubExp1();
					break;

				default:
					giveUp = true;
					break;
				}
			}
		}
	}
}


Location::Location(Location& o)
	: Unary(o.m_oper, o.subExp1->clone())
	, proc(o.proc)
{
}


Unary::~Unary()
{
	// Remember to ;//delete all children
	if (subExp1 != nullptr) {
		// delete subExp1;
	}
}


Binary::~Binary()
{
	if (subExp2 != nullptr) {
		// delete subExp2;
	}

	// Note that the first pointer is destructed in the Exp1 destructor
}


Ternary::~Ternary()
{
	if (subExp3 != nullptr) {
		// delete subExp3;
	}
}


FlagDef::~FlagDef()
{
	// delete rtl;
}


TypeVal::~TypeVal()
{
	// delete val;
}


void Unary::setSubExp1(SharedExp e)
{
	subExp1 = e;
	assert(subExp1);
}


void Binary::setSubExp2(SharedExp e)
{
	if (subExp2 != nullptr) {
		// delete subExp2;
	}

	subExp2 = e;
	assert(subExp1 && subExp2);
}


void Ternary::setSubExp3(SharedExp e)
{
	if (subExp3 != nullptr) {
		// delete subExp3;
	}

	subExp3 = e;
	assert(subExp1 && subExp2 && subExp3);
}


SharedExp Unary::getSubExp1()
{
	assert(subExp1);
	return subExp1;
}


SharedConstExp Unary::getSubExp1() const
{
	assert(subExp1);
	return subExp1;
}


SharedExp& Unary::refSubExp1()
{
	assert(subExp1);
	return subExp1;
}


SharedExp Binary::getSubExp2()
{
	assert(subExp1 && subExp2);
	return subExp2;
}


SharedConstExp Binary::getSubExp2() const
{
	assert(subExp1 && subExp2);
	return subExp2;
}


SharedExp& Binary::refSubExp2()
{
	assert(subExp1 && subExp2);
	return subExp2;
}


SharedExp Ternary::getSubExp3()
{
	assert(subExp1 && subExp2 && subExp3);
	return subExp3;
}


SharedConstExp Ternary::getSubExp3() const
{
	assert(subExp1 && subExp2 && subExp3);
	return subExp3;
}


SharedExp& Ternary::refSubExp3()
{
	assert(subExp1 && subExp2 && subExp3);
	return subExp3;
}


// This to satisfy the compiler (never gets called!)
SharedExp  dummy;
SharedExp& Exp::refSubExp1()
{
	assert(false);
	return dummy;
}


SharedExp& Exp::refSubExp2()
{
	assert(false);
	return dummy;
}


SharedExp& Exp::refSubExp3()
{
	assert(false);
	return dummy;
}


void Binary::commute()
{
	std::swap(subExp1, subExp2);
	assert(subExp1 && subExp2);
}


SharedExp Const::clone() const
{
	// Note: not actually cloning the Type* type pointer. Probably doesn't matter with GC
	return Const::get(*this);
}


SharedExp Terminal::clone() const
{
	return std::make_shared<Terminal>(*this);
}


SharedExp Unary::clone() const
{
	assert(subExp1);
	return std::make_shared<Unary>(m_oper, subExp1->clone());
}


SharedExp Binary::clone() const
{
	assert(subExp1 && subExp2);
	return std::make_shared<Binary>(m_oper, subExp1->clone(), subExp2->clone());
}


SharedExp Ternary::clone() const
{
	assert(subExp1 && subExp2 && subExp3);
	std::shared_ptr<Ternary> c = std::make_shared<Ternary>(m_oper, subExp1->clone(), subExp2->clone(), subExp3->clone());
	return c;
}


SharedExp TypedExp::clone() const
{
	return std::make_shared<TypedExp>(type, subExp1->clone());
}


SharedExp RefExp::clone() const
{
	return RefExp::get(subExp1->clone(), m_def);
}


SharedExp TypeVal::clone() const
{
	return std::make_shared<TypeVal>(val->clone());
}


SharedExp Location::clone() const
{
	return std::make_shared<Location>(m_oper, subExp1->clone(), proc);
}


bool Const::operator==(const Exp& o) const
{
	// Note: the casts of o to Const& are needed, else op is protected! Duh.
	if (o.getOper() == opWild) {
		return true;
	}

	if ((o.getOper() == opWildIntConst) && (m_oper == opIntConst)) {
		return true;
	}

	if ((o.getOper() == opWildStrConst) && (m_oper == opStrConst)) {
		return true;
	}

	if (m_oper != o.getOper()) {
		return false;
	}

	if ((m_conscript && (m_conscript != ((Const&)o).m_conscript)) || ((Const&)o).m_conscript) {
		return false;
	}

	switch (m_oper)
	{
	case opIntConst:
		return u.i == ((Const&)o).u.i;

	case opFltConst:
		return u.d == ((Const&)o).u.d;

	case opStrConst:
		return m_string == ((Const&)o).m_string;

	default:
		LOG << "Operator== invalid operator " << operStrings[m_oper] << "\n";
		assert(false);
	}

	return false;
}


bool Unary::operator==(const Exp& o) const
{
	if (o.getOper() == opWild) {
		return true;
	}

	if ((o.getOper() == opWildRegOf) && (m_oper == opRegOf)) {
		return true;
	}

	if ((o.getOper() == opWildMemOf) && (m_oper == opMemOf)) {
		return true;
	}

	if ((o.getOper() == opWildAddrOf) && (m_oper == opAddrOf)) {
		return true;
	}

	if (m_oper != o.getOper()) {
		return false;
	}

	return *subExp1 == *o.getSubExp1();
}


bool Binary::operator==(const Exp& o) const
{
	assert(subExp1 && subExp2);

	if (o.getOper() == opWild) {
		return true;
	}

	if (nullptr == dynamic_cast<const Binary *>(&o)) {
		return false;
	}

	if (m_oper != ((Binary&)o).m_oper) {
		return false;
	}

	if (!(*subExp1 == *((Binary&)o).getSubExp1())) {
		return false;
	}

	return *subExp2 == *((Binary&)o).getSubExp2();
}


bool Ternary::operator==(const Exp& o) const
{
	if (o.getOper() == opWild) {
		return true;
	}

	if (nullptr == dynamic_cast<const Ternary *>(&o)) {
		return false;
	}

	if (m_oper != ((Ternary&)o).m_oper) {
		return false;
	}

	if (!(*subExp1 == *((Ternary&)o).getSubExp1())) {
		return false;
	}

	if (!(*subExp2 == *((Ternary&)o).getSubExp2())) {
		return false;
	}

	return *subExp3 == *((Ternary&)o).getSubExp3();
}


bool Terminal::operator==(const Exp& o) const
{
	if (m_oper == opWildIntConst) {
		return o.getOper() == opIntConst;
	}

	if (m_oper == opWildStrConst) {
		return o.getOper() == opStrConst;
	}

	if (m_oper == opWildMemOf) {
		return o.getOper() == opMemOf;
	}

	if (m_oper == opWildRegOf) {
		return o.getOper() == opRegOf;
	}

	if (m_oper == opWildAddrOf) {
		return o.getOper() == opAddrOf;
	}

	return((m_oper == opWild) ||  // Wild matches anything
		   (o.getOper() == opWild) || (m_oper == o.getOper()));
}


bool TypedExp::operator==(const Exp& o) const
{
	if (((TypedExp&)o).m_oper == opWild) {
		return true;
	}

	if (((TypedExp&)o).m_oper != opTypedExp) {
		return false;
	}

	// This is the strict type version
	if (*type != *((TypedExp&)o).type) {
		return false;
	}

	return *((Unary *)this)->getSubExp1() == *((Unary&)o).getSubExp1();
}


bool RefExp::operator==(const Exp& o) const
{
	if (o.getOper() == opWild) {
		return true;
	}

	if (o.getOper() != opSubscript) {
		return false;
	}

	if (!(*subExp1 == *o.getSubExp1())) {
		return false;
	}

	// Allow a def of (Statement*)-1 as a wild card
	if (m_def == (Instruction *)-1) {
		return true;
	}

	assert(dynamic_cast<const RefExp *>(&o) != nullptr);

	// Allow a def of nullptr to match a def of an implicit assignment
	if (((RefExp&)o).m_def == (Instruction *)-1) {
		return true;
	}

	if ((m_def == nullptr) && ((RefExp&)o).isImplicitDef()) {
		return true;
	}

	if ((((RefExp&)o).m_def == nullptr) && m_def && m_def->isImplicit()) {
		return true;
	}

	return m_def == ((RefExp&)o).m_def;
}


bool TypeVal::operator==(const Exp& o) const
{
	if (((TypeVal&)o).m_oper == opWild) {
		return true;
	}

	if (((TypeVal&)o).m_oper != opTypeVal) {
		return false;
	}

	return *val == *((TypeVal&)o).val;
}


bool Const::operator<(const Exp& o) const
{
	if (m_oper < o.getOper()) {
		return true;
	}

	if (m_oper > o.getOper()) {
		return false;
	}

	if (m_conscript) {
		if (m_conscript < ((Const&)o).m_conscript) {
			return true;
		}

		if (m_conscript > ((Const&)o).m_conscript) {
			return false;
		}
	}
	else if (((Const&)o).m_conscript) {
		return true;
	}

	switch (m_oper)
	{
	case opIntConst:
		return u.i < ((Const&)o).u.i;

	case opFltConst:
		return u.d < ((Const&)o).u.d;

	case opStrConst:
		return m_string < ((Const&)o).m_string;

	default:
		LOG << "Operator< invalid operator " << operStrings[m_oper] << "\n";
		assert(false);
	}

	return false;
}


bool Terminal::operator<(const Exp& o) const
{
	return(m_oper < o.getOper());
}


bool Unary::operator<(const Exp& o) const
{
	if (m_oper < o.getOper()) {
		return true;
	}

	if (m_oper > o.getOper()) {
		return false;
	}

	return *subExp1 < *((Unary&)o).getSubExp1();
}


bool Binary::operator<(const Exp& o) const
{
	assert(subExp1 && subExp2);

	if (m_oper < o.getOper()) {
		return true;
	}

	if (m_oper > o.getOper()) {
		return false;
	}

	if (*subExp1 < *((Binary&)o).getSubExp1()) {
		return true;
	}

	if (*((Binary&)o).getSubExp1() < *subExp1) {
		return false;
	}

	return *subExp2 < *((Binary&)o).getSubExp2();
}


bool Ternary::operator<(const Exp& o) const
{
	if (m_oper < o.getOper()) {
		return true;
	}

	if (m_oper > o.getOper()) {
		return false;
	}

	if (*subExp1 < *((Ternary&)o).getSubExp1()) {
		return true;
	}

	if (*((Ternary&)o).getSubExp1() < *subExp1) {
		return false;
	}

	if (*subExp2 < *((Ternary&)o).getSubExp2()) {
		return true;
	}

	if (*((Ternary&)o).getSubExp2() < *subExp2) {
		return false;
	}

	return *subExp3 < *((Ternary&)o).getSubExp3();
}


bool TypedExp::operator<<(const Exp& o) const   // Type insensitive
{
	if (m_oper < o.getOper()) {
		return true;
	}

	if (m_oper > o.getOper()) {
		return false;
	}

	return *subExp1 << *((Unary&)o).getSubExp1();
}


bool TypedExp::operator<(const Exp& o) const   // Type sensitive
{
	if (m_oper < o.getOper()) {
		return true;
	}

	if (m_oper > o.getOper()) {
		return false;
	}

	if (*type < *((TypedExp&)o).type) {
		return true;
	}

	if (*((TypedExp&)o).type < *type) {
		return false;
	}

	return *subExp1 < *((Unary&)o).getSubExp1();
}


bool RefExp::operator<(const Exp& o) const
{
	if (opSubscript < o.getOper()) {
		return true;
	}

	if (opSubscript > o.getOper()) {
		return false;
	}

	if (*subExp1 < *((Unary&)o).getSubExp1()) {
		return true;
	}

	if (*((Unary&)o).getSubExp1() < *subExp1) {
		return false;
	}

	// Allow a wildcard def to match any
	if (m_def == (Instruction *)-1) {
		return false; // Not less (equal)
	}

	if (((RefExp&)o).m_def == (Instruction *)-1) {
		return false;
	}

	return m_def < ((RefExp&)o).m_def;
}


bool TypeVal::operator<(const Exp& o) const
{
	if (opTypeVal < o.getOper()) {
		return true;
	}

	if (opTypeVal > o.getOper()) {
		return false;
	}

	return *val < *((TypeVal&)o).val;
}


bool Const::operator*=(const Exp& o) const
{
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	return *this == *other;
}


bool Unary::operator*=(const Exp& o) const
{
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	if (other->getOper() == opWild) {
		return true;
	}

	if ((other->getOper() == opWildRegOf) && (m_oper == opRegOf)) {
		return true;
	}

	if ((other->getOper() == opWildMemOf) && (m_oper == opMemOf)) {
		return true;
	}

	if ((other->getOper() == opWildAddrOf) && (m_oper == opAddrOf)) {
		return true;
	}

	if (m_oper != other->getOper()) {
		return false;
	}

	return *subExp1 *= *other->getSubExp1();
}


bool Binary::operator*=(const Exp& o) const
{
	assert(subExp1 && subExp2);
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	if (other->getOper() == opWild) {
		return true;
	}

	if (m_oper != other->getOper()) {
		return false;
	}

	if (!(*subExp1 *= *other->getSubExp1())) {
		return false;
	}

	return *subExp2 *= *other->getSubExp2();
}


bool Ternary::operator*=(const Exp& o) const
{
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	if (other->getOper() == opWild) {
		return true;
	}

	if (m_oper != other->getOper()) {
		return false;
	}

	if (!(*subExp1 *= *other->getSubExp1())) {
		return false;
	}

	if (!(*subExp2 *= *other->getSubExp2())) {
		return false;
	}

	return *subExp3 *= *other->getSubExp3();
}


bool Terminal::operator*=(const Exp& o) const
{
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	return *this == *other;
}


bool TypedExp::operator*=(const Exp& o) const
{
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	if (other->getOper() == opWild) {
		return true;
	}

	if (other->getOper() != opTypedExp) {
		return false;
	}

	// This is the strict type version
	if (*type != *((TypedExp *)other)->type) {
		return false;
	}

	return *((Unary *)this)->getSubExp1() *= *other->getSubExp1();
}


bool RefExp::operator*=(const Exp& o) const
{
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	return *subExp1 *= *other;
}


bool TypeVal::operator*=(const Exp& o) const
{
	const Exp *other = &o;

	if (o.getOper() == opSubscript) {
		other = o.getSubExp1().get();
	}

	return *this == *other;
}


//    //    //    //
//    Const    //
//    //    //    //

void Const::print(QTextStream& os, bool /*html*/) const
{
	setLexBegin(os.pos());

	switch (m_oper)
	{
	case opIntConst:

		if ((u.i < -1000) || (u.i > 1000)) {
			os << "0x" << QString::number(u.i, 16);
		}
		else {
			os << u.i;
		}

		break;

	case opLongConst:

		if (((long long)u.ll < -1000LL) || ((long long)u.ll > 1000LL)) {
			os << "0x" << QString::number(u.ll, 16) << "LL";
		}
		else {
			os << u.ll << "LL";
		}

		break;

	case opFltConst:
		char buf[64];
		sprintf(buf, "%.4f", u.d); // FIXME: needs an intelligent printer
		os << buf;
		break;

	case opStrConst:
		os << "\"" << m_string << "\"";
		break;

	default:
		LOG << "Const::print invalid operator " << operStrings[m_oper] << "\n";
		assert(false);
	}

	if (m_conscript) {
		os << "\\" << m_conscript << "\\";
	}

#ifdef DUMP_TYPES
	if (type) {
		os << "T(" << type->prints() << ")";
	}
#endif
	setLexEnd(os.pos());
}


void Const::printNoQuotes(QTextStream& os) const
{
	if (m_oper == opStrConst) {
		os << m_string;
	}
	else {
		print(os);
	}
}


//    //    //    //
//    Binary    //
//    //    //    //
void Binary::printr(QTextStream& os, bool html) const
{
	assert(subExp1 && subExp2);

	// The "r" is for recursive: the idea is that we don't want parentheses at the outer level, but a subexpression
	// (recursed from a higher level), we want the parens (at least for standard infix operators)
	switch (m_oper)
	{
	case opSize:
	case opList: // Otherwise, you get (a, (b, (c, d)))
		// There may be others
		// These are the noparen cases
		print(os, html);
		return;

	default:
		break;
	}

	// Normal case: we want the parens
	os << "(";
	this->print(os, html);
	os << ")";
}


void Binary::print(QTextStream& os, bool html) const
{
	assert(subExp1 && subExp2);
	SharedConstExp p1 = getSubExp1();
	SharedConstExp p2 = getSubExp2();

	// Special cases
	switch (m_oper)
	{
	case opSize:
		// This can still be seen after decoding and before type analysis after m[...]
		// *size* is printed after the expression, even though it comes from the first subexpression
		p2->printr(os, html);
		os << "*";
		p1->printr(os, html);
		os << "*";
		return;

	case opFlagCall:
		// The name of the flag function (e.g. ADDFLAGS) should be enough
		std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
		os << "( ";
		p2->printr(os, html);
		os << " )";
		return;

	case opExpTable:
	case opNameTable:

		if (m_oper == opExpTable) {
			os << "exptable(";
		}
		else {
			os << "nametable(";
		}

		os << p1 << ", " << p2 << ")";
		return;

	case opList:
		// Because "," is the lowest precedence operator, we don't need printr here.
		// Also, same as UQBT, so easier to test
		p1->print(os, html);

		if (!p2->isNil()) {
			os << ", ";
		}

		p2->print(os, html);
		return;

	case opMemberAccess:
		p1->print(os, html);
		os << ".";
		std::static_pointer_cast<const Const>(p2)->printNoQuotes(os);
		return;

	case opArrayIndex:
		p1->print(os, html);
		os << "[";
		p2->print(os, html);
		os << "]";
		return;

	default:
		break;
	}

	// Ordinary infix operators. Emit parens around the binary
	if (p1 == nullptr) {
		os << "<nullptr>";
	}
	else {
		p1->printr(os, html);
	}

	switch (m_oper)
	{
	case opPlus:
		os << " + ";
		break;

	case opMinus:
		os << " - ";
		break;

	case opMult:
		os << " * ";
		break;

	case opMults:
		os << " *! ";
		break;

	case opDiv:
		os << " / ";
		break;

	case opDivs:
		os << " /! ";
		break;

	case opMod:
		os << " % ";
		break;

	case opMods:
		os << " %! ";
		break;

	case opFPlus:
		os << " +f ";
		break;

	case opFMinus:
		os << " -f ";
		break;

	case opFMult:
		os << " *f ";
		break;

	case opFDiv:
		os << " /f ";
		break;

	case opPow:
		os << " pow ";
		break; // Raising to power

	case opAnd:
		os << " and ";
		break;

	case opOr:
		os << " or ";
		break;

	case opBitAnd:
		os << " & ";
		break;

	case opBitOr:
		os << " | ";
		break;

	case opBitXor:
		os << " ^ ";
		break;

	case opEquals:
		os << " = ";
		break;

	case opNotEqual:
		os << " ~= ";
		break;

	case opLess:

		if (html) {
			os << " &lt; ";
		}
		else {
			os << " < ";
		}

		break;

	case opGtr:

		if (html) {
			os << " &gt; ";
		}
		else {
			os << " > ";
		}

		break;

	case opLessEq:

		if (html) {
			os << " &lt;= ";
		}
		else {
			os << " <= ";
		}

		break;

	case opGtrEq:

		if (html) {
			os << " &gt;= ";
		}
		else {
			os << " >= ";
		}

		break;

	case opLessUns:

		if (html) {
			os << " &lt;u ";
		}
		else {
			os << " <u ";
		}

		break;

	case opGtrUns:

		if (html) {
			os << " &gt;u ";
		}
		else {
			os << " >u ";
		}

		break;

	case opLessEqUns:

		if (html) {
			os << " &lt;u ";
		}
		else {
			os << " <=u ";
		}

		break;

	case opGtrEqUns:

		if (html) {
			os << " &gt;=u ";
		}
		else {
			os << " >=u ";
		}

		break;

	case opUpper:
		os << " GT ";
		break;

	case opLower:
		os << " LT ";
		break;

	case opShiftL:

		if (html) {
			os << " &lt;&lt; ";
		}
		else {
			os << " << ";
		}

		break;

	case opShiftR:

		if (html) {
			os << " &gt;&gt; ";
		}
		else {
			os << " >> ";
		}

		break;

	case opShiftRA:

		if (html) {
			os << " &gt;&gt;A ";
		}
		else {
			os << " >>A ";
		}

		break;

	case opRotateL:
		os << " rl ";
		break;

	case opRotateR:
		os << " rr ";
		break;

	case opRotateLC:
		os << " rlc ";
		break;

	case opRotateRC:
		os << " rrc ";
		break;

	default:
		LOG << "Binary::print invalid operator " << operStrings[m_oper] << "\n";
		assert(false);
	}

	if (p2 == nullptr) {
		os << "<nullptr>";
	}
	else {
		p2->printr(os, html);
	}
}


//    //    //    //    //
//     Terminal    //
//    //    //    //    //
void Terminal::print(QTextStream& os, bool /*html*/) const
{
	switch (m_oper)
	{
	case opPC:
		os << "%pc";
		break;

	case opFlags:
		os << "%flags";
		break;

	case opFflags:
		os << "%fflags";
		break;

	case opCF:
		os << "%CF";
		break;

	case opZF:
		os << "%ZF";
		break;

	case opOF:
		os << "%OF";
		break;

	case opNF:
		os << "%NF";
		break;

	case opDF:
		os << "%DF";
		break;

	case opAFP:
		os << "%afp";
		break;

	case opAGP:
		os << "%agp";
		break;

	case opWild:
		os << "WILD";
		break;

	case opAnull:
		os << "%anul";
		break;

	case opFpush:
		os << "FPUSH";
		break;

	case opFpop:
		os << "FPOP";
		break;

	case opWildMemOf:
		os << "m[WILD]";
		break;

	case opWildRegOf:
		os << "r[WILD]";
		break;

	case opWildAddrOf:
		os << "a[WILD]";
		break;

	case opWildIntConst:
		os << "WILDINT";
		break;

	case opWildStrConst:
		os << "WILDSTR";
		break;

	case opNil:
		break;

	case opTrue:
		os << "true";
		break;

	case opFalse:
		os << "false";
		break;

	case opDefineAll:
		os << "<all>";
		break;

	default:
		LOG << "Terminal::print invalid operator " << operStrings[m_oper] << "\n";
		assert(false);
	}
}


//    //    //    //
//     Unary    //
//    //    //    //
void Unary::print(QTextStream& os, bool html) const
{
	SharedConstExp p1 = this->getSubExp1();

	switch (m_oper)
	{
	//    //    //    //    //    //    //
	//    x[ subexpression ]    //
	//    //    //    //    //    //    //
	case opRegOf:

		// Make a special case for the very common case of r[intConst]
		if (p1->isIntConst()) {
			os << "r" << std::static_pointer_cast<const Const>(p1)->getInt();
#ifdef DUMP_TYPES
			os << "T(" << std::static_pointer_cast<const Const>(p1)->getType() << ")";
#endif
			break;
		}
		else if (p1->isTemp()) {
			// Just print the temp {   // balance }s
			p1->print(os, html);
			break;
		}
		else {
			os << "r["; // e.g. r[r2]
			// Use print, not printr, because this is effectively the top level again (because the [] act as
			// parentheses)
			p1->print(os, html);
		}

		os << "]";
		break;

	case opMemOf:
	case opAddrOf:
	case opVar:
	case opTypeOf:
	case opKindOf:

		switch (m_oper)
		{
		case opMemOf:
			os << "m[";
			break;

		case opAddrOf:
			os << "a[";
			break;

		case opVar:
			os << "v[";
			break;

		case opTypeOf:
			os << "T[";
			break;

		case opKindOf:
			os << "K[";
			break;

		default:
			break; // Suppress compiler warning
		}

		if (m_oper == opVar) {
			std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
		}
		// Use print, not printr, because this is effectively the top level again (because the [] act as
		// parentheses)
		else {
			p1->print(os, html);
		}

		os << "]";
#ifdef DUMP_TYPES
		os << "T(" << std::static_pointer_cast<const Const>(p1)->getType() << ")";
#endif
		break;

	//    //    //    //    //    //    //
	//      Unary operators    //
	//    //    //    //    //    //    //

	case opNot:
	case opLNot:
	case opNeg:
	case opFNeg:

		if (m_oper == opNot) {
			os << "~";
		}
		else if (m_oper == opLNot) {
			os << "L~";
		}
		else if (m_oper == opFNeg) {
			os << "~f ";
		}
		else {
			os << "-";
		}

		p1->printr(os, html);
		return;

	case opSignExt:
		p1->printr(os, html);
		os << "!"; // Operator after expression
		return;

	//    //    //    //    //    //    //    //
	//    Function-like operators //
	//    //    //    //    //    //    //    //

	case opSQRTs:
	case opSQRTd:
	case opSQRTq:
	case opSqrt:
	case opSin:
	case opCos:
	case opTan:
	case opArcTan:
	case opLog2:
	case opLog10:
	case opLoge:
	case opPow:
	case opMachFtr:
	case opSuccessor:

		switch (m_oper)
		{
		case opSQRTs:
			os << "SQRTs(";
			break;

		case opSQRTd:
			os << "SQRTd(";
			break;

		case opSQRTq:
			os << "SQRTq(";
			break;

		case opSqrt:
			os << "sqrt(";
			break;

		case opSin:
			os << "sin(";
			break;

		case opCos:
			os << "cos(";
			break;

		case opTan:
			os << "tan(";
			break;

		case opArcTan:
			os << "arctan(";
			break;

		case opLog2:
			os << "log2(";
			break;

		case opLog10:
			os << "log10(";
			break;

		case opLoge:
			os << "loge(";
			break;

		case opExecute:
			os << "execute(";
			break;

		case opMachFtr:
			os << "machine(";
			break;

		case opSuccessor:
			os << "succ(";
			break;

		default:
			break; // For warning
		}

		p1->printr(os, html);
		os << ")";
		return;

	//    Misc    //
	case opSgnEx: // Different because the operator appears last
		p1->printr(os, html);
		os << "! ";
		return;

	case opTemp:

		if (p1->getOper() == opWildStrConst) {
			assert(p1->isTerminal());
			os << "t[";
			std::static_pointer_cast<const Terminal>(p1)->print(os);
			os << "]";
			return;
		}

	// Temp: just print the string, no quotes
	case opGlobal: // [[clang::fallthrough]];
	case opLocal:
	case opParam:
		// Print a more concise form than param["foo"] (just foo)
		std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
		return;

	case opInitValueOf:
		p1->printr(os, html);
		os << "'";
		return;

	case opPhi:
		os << "phi(";
		p1->print(os, html);
		os << ")";
		return;

	case opFtrunc:
		os << "ftrunc(";
		p1->print(os, html);
		os << ")";
		return;

	case opFabs:
		os << "fabs(";
		p1->print(os, html);
		os << ")";
		return;

	default:
		LOG << "Unary::print invalid operator " << operStrings[m_oper] << "\n";
		assert(false);
	}
}


//    //    //    //
//    Ternary //
//    //    //    //
void Ternary::printr(QTextStream& os, bool /*html*/) const
{
	// The function-like operators don't need parentheses
	switch (m_oper)
	{
	// The "function-like" ternaries
	case opTruncu:
	case opTruncs:
	case opZfill:
	case opSgnEx:
	case opFsize:
	case opItof:
	case opFtoi:
	case opFround:
	case opFtrunc:
	case opOpTable:
		// No paren case
		print(os);
		return;

	default:
		break;
	}

	// All other cases, we use the parens
	os << "(" << this << ")";
}


void Ternary::print(QTextStream& os, bool html) const
{
	SharedConstExp p1 = this->getSubExp1();
	SharedConstExp p2 = this->getSubExp2();
	SharedConstExp p3 = this->getSubExp3();

	switch (m_oper)
	{
	// The "function-like" ternaries
	case opTruncu:
	case opTruncs:
	case opZfill:
	case opSgnEx:
	case opFsize:
	case opItof:
	case opFtoi:
	case opFround:
	case opFtrunc:
	case opOpTable:

		switch (m_oper)
		{
		case opTruncu:
			os << "truncu(";
			break;

		case opTruncs:
			os << "truncs(";
			break;

		case opZfill:
			os << "zfill(";
			break;

		case opSgnEx:
			os << "sgnex(";
			break;

		case opFsize:
			os << "fsize(";
			break;

		case opItof:
			os << "itof(";
			break;

		case opFtoi:
			os << "ftoi(";
			break;

		case opFround:
			os << "fround(";
			break;

		case opFtrunc:
			os << "ftrunc(";
			break;

		case opOpTable:
			os << "optable(";
			break;

		default:
			break; // For warning
		}

		// Use print not printr here, since , has the lowest precendence of all.
		// Also it makes it the same as UQBT, so it's easier to test
		if (p1) {
			p1->print(os, html);
		}
		else {
			os << "<nullptr>";
		}

		os << ",";

		if (p2) {
			p2->print(os, html);
		}
		else {
			os << "<nullptr>";
		}

		os << ",";

		if (p3) {
			p3->print(os, html);
		}
		else {
			os << "<nullptr>";
		}

		os << ")";
		return;

	default:
		break;
	}

	// Else must be ?: or @ (traditional ternary operators)
	if (p1) {
		p1->printr(os, html);
	}
	else {
		os << "<nullptr>";
	}

	if (m_oper == opTern) {
		os << " ? ";

		if (p2) {
			p2->printr(os, html);
		}
		else {
			os << "<nullptr>";
		}

		os << " : "; // Need wide spacing here

		if (p3) {
			p3->print(os, html);
		}
		else {
			os << "<nullptr>";
		}
	}
	else if (m_oper == opAt) {
		os << "@";

		if (p2) {
			p2->printr(os, html);
		}
		else {
			os << "nullptr>";
		}

		os << ":";

		if (p3) {
			p3->printr(os, html);
		}
		else {
			os << "nullptr>";
		}
	}
	else {
		LOG << "Ternary::print invalid operator " << operStrings[m_oper] << "\n";
		assert(false);
	}
}


//    //    //    //
// TypedExp //
//    //    //    //
void TypedExp::print(QTextStream& os, bool html) const
{
	os << " ";
	type->starPrint(os);
	SharedConstExp p1 = this->getSubExp1();
	p1->print(os, html);
}


//    //    //    //
//    RefExp    //
//    //    //    //
void RefExp::print(QTextStream& os, bool html) const
{
	if (subExp1) {
		subExp1->print(os, html);
	}
	else {
		os << "<nullptr>";
	}

	if (html) {
		os << "<sub>";
	}
	else {
		os << "{";
	}

	if (m_def == (Instruction *)-1) {
		os << "WILD";
	}
	else if (m_def) {
		if (html) {
			os << "<a href=\"#stmt" << m_def->getNumber() << "\">";
		}

		m_def->printNum(os);

		if (html) {
			os << "</a>";
		}
	}
	else {
		os << "-"; // So you can tell the difference with {0}
	}

	if (html) {
		os << "</sub>";
	}
	else {
		os << "}";
	}
}


//    //    //    //
// TypeVal    //
//    //    //    //
void TypeVal::print(QTextStream& os, bool /*html*/) const
{
	if (val) {
		os << "<" << val->getCtype() << ">";
	}
	else {
		os << "<nullptr>";
	}
}


/***************************************************************************/ /**
 *
 * \brief        Print to a static string (for debugging)
 * \returns            Address of the static buffer
 ******************************************************************************/
char *Exp::prints()
{
	QString     tgt;
	QTextStream ost(&tgt);

	print(ost);
	strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
	debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
	return debug_buffer;
}


void Exp::dump()
{
	QTextStream ost(stderr);

	print(ost);
}


/***************************************************************************/ /**
 *
 * \brief Create a dotty file (use dotty to display the file; search the web for "graphviz").
 *        Mainly for debugging
 * \param name - Name of the file to create
 *
 ******************************************************************************/
void Exp::createDotFile(const char *name)
{
	QFile fl(name);

	if (!fl.open(QFile::WriteOnly)) {
		LOG << "Could not open " << name << " to write dotty file\n";
		return;
	}

	QTextStream of(&fl);
	of << "digraph Exp {\n";
	appendDotFile(of);
	of << "}";
}


//    //    //    //
//    Const    //
//    //    //    //
void Const::appendDotFile(QTextStream& of)
{
	// We define a unique name for each node as "e123456" if the address of "this" == 0x123456
	of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
	of << operStrings[m_oper] << "\\n" << ADDRESS::host_ptr(this) << " | ";

	switch (m_oper)
	{
	case opIntConst:
		of << u.i;
		break;

	case opFltConst:
		of << u.d;
		break;

	case opStrConst:
		of << "\\\"" << m_string << "\\\"";
		break;

	// Might want to distinguish this better, e.g. "(func*)myProc"
	case opFuncConst:
		of << u.pp->getName();
		break;

	default:
		break;
	}

	of << " }\"];\n";
}


//    //    //    //
// Terminal //
//    //    //    //
void Terminal::appendDotFile(QTextStream& of)
{
	of << "e" << ADDRESS::host_ptr(this) << " [shape=parallelogram,label=\"";

	if (m_oper == opWild) {
		// Note: value is -1, so can't index array
		of << "WILD";
	}
	else {
		of << operStrings[m_oper];
	}

	of << "\\n" << ADDRESS::host_ptr(this);
	of << "\"];\n";
}


//    //    //    //
//    Unary    //
//    //    //    //
void Unary::appendDotFile(QTextStream& of)
{
	// First a node for this Unary object
	of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
	// The (int) cast is to print the address, not the expression!
	of << operStrings[m_oper] << "\\n" << ADDRESS::host_ptr(this) << " | ";
	of << "<p1>";
	of << " }\"];\n";

	// Now recurse to the subexpression.
	subExp1->appendDotFile(of);

	// Finally an edge for the subexpression
	of << "e" << ADDRESS::host_ptr(this) << "->e" << ADDRESS::host_ptr(subExp1.get()) << ";\n";
}


//    //    //    //
//    Binary    //
//    //    //    //
void Binary::appendDotFile(QTextStream& of)
{
	// First a node for this Binary object
	of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
	of << operStrings[m_oper] << "\\n" << ADDRESS::host_ptr(this) << " | ";
	of << "{<p1> | <p2>}";
	of << " }\"];\n";
	subExp1->appendDotFile(of);
	subExp2->appendDotFile(of);
	// Now an edge for each subexpression
	of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1.get()) << ";\n";
	of << "e" << ADDRESS::host_ptr(this) << ":p2->e" << ADDRESS::host_ptr(subExp2.get()) << ";\n";
}


//    //    //    //
//    Ternary //
//    //    //    //
void Ternary::appendDotFile(QTextStream& of)
{
	// First a node for this Ternary object
	of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
	of << operStrings[m_oper] << "\\n0x" << ADDRESS::host_ptr(this) << " | ";
	of << "{<p1> | <p2> | <p3>}";
	of << " }\"];\n";
	subExp1->appendDotFile(of);
	subExp2->appendDotFile(of);
	subExp3->appendDotFile(of);
	// Now an edge for each subexpression
	of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1.get()) << ";\n";
	of << "e" << ADDRESS::host_ptr(this) << ":p2->e" << ADDRESS::host_ptr(subExp2.get()) << ";\n";
	of << "e" << ADDRESS::host_ptr(this) << ":p3->e" << ADDRESS::host_ptr(subExp3.get()) << ";\n";
}


//    //    //    //
// TypedExp //
//    //    //    //
void TypedExp::appendDotFile(QTextStream& of)
{
	of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
	of << "opTypedExp\\n" << ADDRESS::host_ptr(this) << " | ";
	// Just display the C type for now
	of << type->getCtype() << " | <p1>";
	of << " }\"];\n";
	subExp1->appendDotFile(of);
	of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1.get()) << ";\n";
}


//    //    //    //
//    FlagDef //
//    //    //    //
void FlagDef::appendDotFile(QTextStream& of)
{
	of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
	of << "opFlagDef \\n" << ADDRESS::host_ptr(this) << "| ";
	// Display the RTL as "RTL <r1> <r2>..." vertically (curly brackets)
	of << "{ RTL ";
	int n = rtl->size();

	for (int i = 0; i < n; i++) {
		of << "| <r" << i << "> ";
	}

	of << "} | <p1> }\"];\n";
	subExp1->appendDotFile(of);
	of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1.get()) << ";\n";
}


bool Exp::isRegOfK()
{
	if (m_oper != opRegOf) {
		return false;
	}

	return ((Unary *)this)->getSubExp1()->getOper() == opIntConst;
}


bool Exp::isRegN(int N) const
{
	if (m_oper != opRegOf) {
		return false;
	}

	SharedConstExp sub = ((const Unary *)this)->getSubExp1();
	return(sub->getOper() == opIntConst && std::static_pointer_cast<const Const>(sub)->getInt() == N);
}


bool Exp::isAfpTerm()
{
	auto cur = shared_from_this();

	if (m_oper == opTypedExp) {
		cur = getSubExp1();
	}

	SharedExp p;

	if ((cur->getOper() == opAddrOf) && ((p = cur->getSubExp1()), (p->getOper() == opMemOf))) {
		cur = p->getSubExp1();
	}

	OPER curOp = cur->getOper();

	if (curOp == opAFP) {
		return true;
	}

	if ((curOp != opPlus) && (curOp != opMinus)) {
		return false;
	}

	// cur must be a Binary* now
	OPER subOp1 = cur->getSubExp1()->getOper();
	OPER subOp2 = cur->getSubExp2()->getOper();
	return((subOp1 == opAFP) && (subOp2 == opIntConst));
}


int Exp::getVarIndex()
{
	assert(m_oper == opVar);
	SharedExp sub = this->getSubExp1();
	return std::static_pointer_cast<const Const>(sub)->getInt();
}


SharedExp Exp::getGuard()
{
	if (m_oper == opGuard) {
		return getSubExp1();
	}

	return nullptr;
}


SharedExp Exp::match(const SharedConstExp& pattern)
{
	if (*this == *pattern) {
		return Terminal::get(opNil);
	}

	if (pattern->getOper() == opVar) {
		return Binary::get(opList, Binary::get(opEquals, pattern->clone(), this->clone()), Terminal::get(opNil));
	}

	return nullptr;
}


SharedExp Unary::match(const SharedConstExp& pattern)
{
	assert(subExp1);

	if (m_oper == pattern->getOper()) {
		return subExp1->match(pattern->getSubExp1());
	}

	return Exp::match(pattern);
}


SharedExp Binary::match(const SharedConstExp& pattern)
{
	assert(subExp1 && subExp2);

	if (m_oper != pattern->getOper()) {
		return Exp::match(pattern);
	}

	SharedExp b_lhs = subExp1->match(pattern->getSubExp1());

	if (b_lhs == nullptr) {
		return nullptr;
	}

	SharedExp b_rhs = subExp2->match(pattern->getSubExp2());

	if (b_rhs == nullptr) {
		return nullptr;
	}

	if (b_lhs->getOper() == opNil) {
		return b_rhs;
	}

	if (b_rhs->getOper() == opNil) {
		return b_lhs;
	}

#if 0
	LOG << "got lhs list " << b_lhs << " and rhs list " << b_rhs << "\n";
#endif
	SharedExp result = Terminal::get(opNil);

	// TODO: verify that adding (l &&) is not violating unwritten validity assertion
	for (SharedExp l = b_lhs; l && (l->getOper() != opNil); l = l->getSubExp2()) {
		for (SharedExp r = b_rhs; r && r->getOper() != opNil; r = r->getSubExp2()) {
			if ((*l->getSubExp1()->getSubExp1() == *r->getSubExp1()->getSubExp1()) &&
				!(*l->getSubExp1()->getSubExp2() == *r->getSubExp1()->getSubExp2())) {
#if 0
				LOG << "disagreement in match: " << l->getSubExp1()->getSubExp2() << " != " <<
					r->getSubExp1()->getSubExp2() << "\n";
#endif
				return nullptr; // must be agreement between LHS and RHS
			}
			else {
				result = Binary::get(opList, l->getSubExp1()->clone(), result);
			}
		}
	}

	for (SharedExp r = b_rhs; r->getOper() != opNil; r = r->getSubExp2()) {
		result = Binary::get(opList, r->getSubExp1()->clone(), result);
	}

	return result;
}


SharedExp RefExp::match(const SharedConstExp& pattern)
{
	SharedExp r = Unary::match(pattern);

	//      if (r)
	return r;

	/*      r = subExp1->match(pattern);
	 *  if (r) {
	 *          bool change;
	 *          r = r->searchReplaceAll(subExp1->clone(), this->clone(), change);
	 *          return r;
	 *  }
	 *  return Exp::match(pattern); */
}


#if 0 // Suspect ADHOC TA only
Exp *TypeVal::match(SharedExp pattern)
{
	if (op == pattern->getOper()) {
		return val->match(pattern->getType());
	}

	return Exp::match(pattern);
}


#endif

static QRegularExpression variableRegexp("[a-zA-Z0-9]+");

// TODO use regexp ?
#define ISVARIABLE_S(x)	\
	(strspn((x.c_str()), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == (x).length())
// #define DEBUG_MATCH

int tlstrchr(const QString& str, char ch)
{
	static QMap<QChar, QChar> braces { {
										   '[', ']'
									   }, {
										   '{', '}'
									   }, {
										   '(', ')'
									   }
	};
	int i = 0, e = str.length();

	for ( ; i < e; ++i) {
		if (str[i].toLatin1() == ch) {
			return i;
		}

		if (braces.contains(str[i])) {
			QChar end_brace = braces[str[i]];
			++i; // from next char

			for ( ; i < e; ++i) {
				if (str[i] == end_brace) {
					break;
				}
			}
		}
	}

	if (i == e) {
		return -1;
	}

	return i;
}


bool Exp::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	// most obvious
	QString     tgt;
	QTextStream ostr(&tgt);

	print(ostr);

	if (tgt == pattern) {
		return true;
	}

	assert((pattern.lastIndexOf(variableRegexp) == 0) == ISVARIABLE_S(pattern.toStdString()));

	// alright, is pattern an acceptable variable?
	if (pattern.lastIndexOf(variableRegexp) == 0) {
		bindings[pattern] = shared_from_this();
		return true;
	}

	// no, fail
	return false;
}


bool Unary::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	if (Exp::match(pattern, bindings)) {
		return true;
	}

#ifdef DEBUG_MATCH
	LOG << "unary::match " << this << " to " << pattern << ".\n";
#endif

	if ((m_oper == opAddrOf) && pattern.startsWith("a[") && pattern.endsWith(']')) {
		return subExp1->match(pattern.mid(2, pattern.size() - 1), bindings); // eliminate 'a[' and ']'
	}

	return false;
}


bool Binary::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	if (Exp::match(pattern, bindings)) {
		return true;
	}

#ifdef DEBUG_MATCH
	LOG << "binary::match " << this << " to " << pattern << ".\n";
#endif

	if ((m_oper == opMemberAccess) && (-1 != tlstrchr(pattern, '.'))) {
		QString sub1        = pattern;
		int     split_point = tlstrchr(sub1, '.');
		QString follow      = sub1.right(sub1.length() - split_point);
		sub1 = sub1.left(split_point);

		if (subExp1->match(sub1, bindings)) {
			assert(subExp2->isStrConst());

			if (follow == std::static_pointer_cast<const Const>(subExp2)->getStr()) {
				return true;
			}

			if ((follow.lastIndexOf(variableRegexp) == 0)) {
				bindings[follow] = subExp2;
				return true;
			}
		}
	}

	if (m_oper == opArrayIndex) {
		if (!pattern.endsWith(']')) {
			return false;
		}

		QString sub1 = pattern;
		QString sub2 = sub1.mid(sub1.lastIndexOf('[') + 1);

		if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings)) {
			return true;
		}
	}

	if ((m_oper == opPlus) && (-1 != tlstrchr(pattern, '+'))) {
		int     splitpoint = tlstrchr(pattern, '+');
		QString sub1       = pattern.left(splitpoint);
		QString sub2       = pattern.mid(splitpoint + 1).trimmed();

		if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings)) {
			return true;
		}
	}

	if ((m_oper == opMinus) && (-1 != tlstrchr(pattern, '-'))) {
		int     splitpoint = tlstrchr(pattern, '-');
		QString sub1       = pattern.left(splitpoint);
		QString sub2       = pattern.mid(splitpoint + 1).trimmed();

		if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings)) {
			return true;
		}
	}

	return false;
}


bool Ternary::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	if (Exp::match(pattern, bindings)) {
		return true;
	}

#ifdef DEBUG_MATCH
	LOG << "ternary::match " << this << " to " << pattern << ".\n";
#endif
	return false;
}


bool RefExp::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	if (Exp::match(pattern, bindings)) {
		return true;
	}

#ifdef DEBUG_MATCH
	LOG << "refexp::match " << this << " to " << pattern << ".\n";
#endif

	if (pattern.endsWith('}')) {
		if ((pattern[pattern.size() - 2] == '-') && (m_def == nullptr)) {
			return subExp1->match(pattern.left(pattern.size() - 3), bindings); // remove {-}
		}

		int end = pattern.lastIndexOf('{');

		if (end != -1) {
			// "prefix {number ...}" -> number matches first def ?
			if (pattern.midRef(end + 1).toInt() == m_def->getNumber()) {
				// match "prefix"
				return subExp1->match(pattern.left(end - 1), bindings);
			}
		}
	}

	return false;
}


bool Const::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	if (Exp::match(pattern, bindings)) {
		return true;
	}

#ifdef DEBUG_MATCH
	LOG << "const::match " << this << " to " << pattern << ".\n";
#endif
	return false;
}


bool Terminal::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	if (Exp::match(pattern, bindings)) {
		return true;
	}

#ifdef DEBUG_MATCH
	LOG << "terminal::match " << this << " to " << pattern << ".\n";
#endif
	return false;
}


bool Location::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
	if (Exp::match(pattern, bindings)) {
		return true;
	}

#ifdef DEBUG_MATCH
	LOG << "location::match " << this << " to " << pattern << ".\n";
#endif

	if ((m_oper == opMemOf) || (m_oper == opRegOf)) {
		if ((m_oper == opRegOf) && !pattern.startsWith("r[")) {
			return false;
		}

		if ((m_oper == opMemOf) && !pattern.startsWith("m[")) {
			return false;
		}

		if (!pattern.endsWith(']')) {
			return false;
		}

		return subExp1->match(pattern.mid(2), bindings); // shouldn't this cut the last ']' ??
	}

	return false;
}


void Exp::doSearch(const Exp& search, SharedExp& pSrc, std::list<SharedExp *>& li, bool once)
{
	bool compare;

	compare = (search == *pSrc);

	if (compare) {
		li.push_back(&pSrc); // Success

		if (once) {
			return; // No more to do
		}
	}

	// Either want to find all occurrences, or did not match at this level
	// Recurse into children, unless a matching opSubscript
	if (!compare || (pSrc->m_oper != opSubscript)) {
		pSrc->doSearchChildren(search, li, once);
	}
}


void Exp::doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once)
{
	Q_UNUSED(search);
	Q_UNUSED(li);
	Q_UNUSED(once);
	// Const and Terminal do not override this
}


void Unary::doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once)
{
	if (m_oper != opInitValueOf) { // don't search child
		doSearch(search, subExp1, li, once);
	}
}


void Binary::doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once)
{
	assert(subExp1 && subExp2);
	doSearch(search, subExp1, li, once);

	if (once && li.size()) {
		return;
	}

	doSearch(search, subExp2, li, once);
}


void Ternary::doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once)
{
	doSearch(search, subExp1, li, once);

	if (once && li.size()) {
		return;
	}

	doSearch(search, subExp2, li, once);

	if (once && li.size()) {
		return;
	}

	doSearch(search, subExp3, li, once);
}


SharedExp Exp::searchReplace(const Exp& search, const SharedExp& replace, bool& change)
{
	return searchReplaceAll(search, replace, change, true);
}


SharedExp Exp::searchReplaceAll(const Exp& search, const SharedExp& replace, bool& change, bool once /* = false */)
{
	// TODO: consider working on base object, and only in case when we find the search, use clone call to return the
	// new object ?
	if (this == &search) { // TODO: WAT ?
		change = true;
		return replace->clone();
	}

	std::list<SharedExp *> li;
	SharedExp              top = shared_from_this(); // top may change; that's why we have to return it
	doSearch(search, top, li, false);

	for (auto it = li.begin(); it != li.end(); it++) {
		SharedExp *pp = *it;
		*pp = replace->clone(); // Do the replacement

		if (once) {
			change = true;
			return top;
		}
	}

	change = (li.size() != 0);
	return top;
}


bool Exp::search(const Exp& search, SharedExp& result)
{
	std::list<SharedExp *> li;
	result = nullptr; // In case it fails; don't leave it unassigned
	// The search requires a reference to a pointer to this object.
	// This isn't needed for searches, only for replacements, but we want to re-use the same search routine
	SharedExp top = shared_from_this();
	doSearch(search, top, li, false);

	if (li.size()) {
		result = *li.front();
		return true;
	}

	return false;
}


bool Exp::searchAll(const Exp& search, std::list<SharedExp>& result)
{
	std::list<SharedExp *> li;
	// result.clear();    // No! Useful when searching for more than one thing
	// (add to the same list)
	// The search requires a reference to a pointer to this object.
	// This isn't needed for searches, only for replacements, but we want to re-use the same search routine
	SharedExp pSrc = shared_from_this();
	doSearch(search, pSrc, li, false);

	for (auto it : li) {
		// li is list of pointers to SharedExp ; result is list of SharedExp
		result.push_back(*it);
	}

	return not li.empty();
}


void Exp::partitionTerms(std::list<SharedExp>& positives, std::list<SharedExp>& negatives, std::vector<int>& integers,
						 bool negate)
{
	SharedExp p1, p2;

	switch (m_oper)
	{
	case opPlus:
		p1 = getSubExp1();
		p2 = getSubExp2();
		p1->partitionTerms(positives, negatives, integers, negate);
		p2->partitionTerms(positives, negatives, integers, negate);
		break;

	case opMinus:
		p1 = getSubExp1();
		p2 = getSubExp2();
		p1->partitionTerms(positives, negatives, integers, negate);
		p2->partitionTerms(positives, negatives, integers, !negate);
		break;

	case opTypedExp:
		p1 = getSubExp1();
		p1->partitionTerms(positives, negatives, integers, negate);
		break;

	case opIntConst:
		{
			int k = static_cast<Const *>(this)->getInt();

			if (negate) {
				integers.push_back(-k);
			}
			else {
				integers.push_back(k);
			}

			break;
		}

	default:

		// These can be any other expression tree
		if (negate) {
			negatives.push_back(shared_from_this());
		}
		else {
			positives.push_back(shared_from_this());
		}
	}
}


SharedExp Unary::simplifyArith()
{
	if ((m_oper == opMemOf) || (m_oper == opRegOf) || (m_oper == opAddrOf) || (m_oper == opSubscript)) {
		// assume we want to simplify the subexpression
		subExp1 = subExp1->simplifyArith();
	}

	return shared_from_this(); // Else, do nothing
}


SharedExp Ternary::simplifyArith()
{
	subExp1 = subExp1->simplifyArith();
	subExp2 = subExp2->simplifyArith();
	subExp3 = subExp3->simplifyArith();
	return shared_from_this();
}


SharedExp Binary::simplifyArith()
{
	assert(subExp1 && subExp2);
	subExp1 = subExp1->simplifyArith(); // FIXME: does this make sense?
	subExp2 = subExp2->simplifyArith(); // FIXME: ditto

	if ((m_oper != opPlus) && (m_oper != opMinus)) {
		return shared_from_this();
	}

	// Partition this expression into positive non-integer terms, negative
	// non-integer terms and integer terms.
	std::list<SharedExp> positives;
	std::list<SharedExp> negatives;
	std::vector<int>     integers;
	partitionTerms(positives, negatives, integers, false);

	// Now reduce these lists by cancelling pairs
	// Note: can't improve this algorithm using multisets, since can't instantiate multisets of type Exp (only Exp*).
	// The Exp* in the multisets would be sorted by address, not by value of the expression.
	// So they would be unsorted, same as lists!
	std::list<SharedExp>::iterator pp = positives.begin();
	std::list<SharedExp>::iterator nn = negatives.begin();

	while (pp != positives.end()) {
		bool inc = true;

		while (nn != negatives.end()) {
			if (**pp == **nn) {
				// A positive and a negative that are equal; therefore they cancel
				pp  = positives.erase(pp); // Erase the pointers, not the Exps
				nn  = negatives.erase(nn);
				inc = false;               // Don't increment pp now
				break;
			}

			nn++;
		}

		if (pp == positives.end()) {
			break;
		}

		if (inc) {
			pp++;
		}
	}

	// Summarise the set of integers to a single number.
	int sum = std::accumulate(integers.begin(), integers.end(), 0);

	// Now put all these elements back together and return the result
	if (positives.size() == 0) {
		if (negatives.size() == 0) {
			return Const::get(sum);
		}
		else {
			// No positives, some negatives. sum - Acc
			return Binary::get(opMinus, Const::get(sum), Exp::accumulate(negatives));
		}
	}

	if (negatives.size() == 0) {
		// Positives + sum
		if (sum == 0) {
			// Just positives
			return Exp::accumulate(positives);
		}
		else {
			OPER _op = opPlus;

			if (sum < 0) {
				_op = opMinus;
				sum = -sum;
			}

			return Binary::get(_op, Exp::accumulate(positives), Const::get(sum));
		}
	}

	// Some positives, some negatives
	if (sum == 0) {
		// positives - negatives
		return Binary::get(opMinus, Exp::accumulate(positives), Exp::accumulate(negatives));
	}

	// General case: some positives, some negatives, a sum
	OPER _op = opPlus;

	if (sum < 0) {
		_op = opMinus; // Return (pos - negs) - sum
		sum = -sum;
	}

	return Binary::get(_op, Binary::get(opMinus, Exp::accumulate(positives), Exp::accumulate(negatives)),
					   Const::get(sum));
}


SharedExp Exp::accumulate(std::list<SharedExp>& exprs)
{
	int n = exprs.size();

	if (n == 0) {
		return Const::get(0);
	}

	if (n == 1) {
		return exprs.front()->clone();
	}

	std::list<SharedExp> cloned_list;

	for (const SharedExp& v : exprs) {
		cloned_list.push_back(v->clone());
	}

	SharedExp last_val = cloned_list.back();
	cloned_list.pop_back();
	auto res = Binary::get(opPlus, cloned_list.back(), last_val);
	cloned_list.pop_back();

	while (not cloned_list.empty()) {
		res = Binary::get(opPlus, cloned_list.back(), res);
		cloned_list.pop_back();
	}

	return res;
}


#define DEBUG_SIMP    0  // Set to 1 to print every change
SharedExp Exp::simplify()
{
#if DEBUG_SIMP
	SharedExp save = clone();
#endif
	bool      bMod = false; // True if simplified at this or lower level
	SharedExp res  = shared_from_this();

	// res = ExpTransformer::applyAllTo(res, bMod);
	// return res;
	do {
		bMod = false;
		// SharedExp before = res->clone();
		res = res->polySimplify(bMod); // Call the polymorphic simplify

		/*      if (bMod) {
		 *              LOG << "polySimplify hit: " << before << " to " << res << "\n";
		 *              // polySimplify is now redundant, if you see this in the log you need to update one of the files
		 * in the
		 *              // transformations directory to include a rule for the reported transform.
		 *      } */
	} while (bMod);                    // If modified at this (or a lower) level, redo

// The below is still important. E.g. want to canonicalise sums, so we know that a + K + b is the same as a + b + K
// No! This slows everything down, and it's slow enough as it is. Call only where needed:
// res = res->simplifyArith();
#if DEBUG_SIMP
	if (!(*res == *save)) {
		std::cout << "simplified " << save << "  to  " << res << "\n";
	}
	// delete save;
#endif
	return res;
}


SharedExp Unary::polySimplify(bool& bMod)
{
	SharedExp res(shared_from_this());

	subExp1 = subExp1->polySimplify(bMod);

	if ((m_oper == opNot) || (m_oper == opLNot)) {
		switch (subExp1->getOper())
		{
		case opEquals:
			res = res->getSubExp1();
			res->setOper(opNotEqual);
			bMod = true;
			return res;

		case opNotEqual:
			res = res->getSubExp1();
			res->setOper(opEquals);
			bMod = true;
			return res;

		case opLess:
			res = res->getSubExp1();
			res->setOper(opGtrEq);
			bMod = true;
			return res;

		case opLessEq:
			res = res->getSubExp1();
			res->setOper(opGtr);
			bMod = true;
			return res;

		case opGtr:
			res = res->getSubExp1();
			res->setOper(opLessEq);
			bMod = true;
			return res;

		case opGtrEq:
			res = res->getSubExp1();
			res->setOper(opLess);
			bMod = true;
			return res;

		case opLessUns:
			res = res->getSubExp1();
			res->setOper(opGtrEqUns);
			bMod = true;
			return res;

		case opLessEqUns:
			res = res->getSubExp1();
			res->setOper(opGtrUns);
			bMod = true;
			return res;

		case opGtrUns:
			res = res->getSubExp1();
			res->setOper(opLessEqUns);
			bMod = true;
			return res;

		case opGtrEqUns:
			res = res->getSubExp1();
			res->setOper(opLessUns);
			bMod = true;
			return res;

		default:
			break;
		}
	}

	switch (m_oper)
	{
	case opNeg:
	case opNot:
	case opLNot:
	case opSize:
		{
			OPER subOP = subExp1->getOper();

			if (subOP == opIntConst) {
				// -k, ~k, or !k
				OPER op2 = m_oper;
				res = res->getSubExp1();
				int k = std::static_pointer_cast<Const>(res)->getInt();

				switch (op2)
				{
				case opNeg:
					k = -k;
					break;

				case opNot:
					k = ~k;
					break;

				case opLNot:
					k = !k;
					break;

				case opSize: /* No change required */
					break;

				default:
					break;
				}

				std::static_pointer_cast<Const>(res)->setInt(k);
				bMod = true;
			}
			else if (m_oper == subOP) {
				res  = res->getSubExp1();
				res  = res->getSubExp1();
				bMod = true;
				break;
			}
		}
		break;

	case opAddrOf:

		// check for a[m[x]], becomes x
		if (subExp1->getOper() == opMemOf) {
			res  = res->getSubExp1();
			res  = res->getSubExp1();
			bMod = true;
			return res;
		}

		break;

	case opMemOf:
	case opRegOf:
		subExp1 = subExp1->polySimplify(bMod);
		// The below IS bad now. It undoes the simplification of
		// m[r29 + -4] to m[r29 - 4]
		// If really needed, do another polySimplify, or swap the order
		// subExp1 = subExp1->simplifyArith();        // probably bad
		break;

	default:
		break;
	}

	return res;
}


SharedExp accessMember(SharedExp parent, const std::shared_ptr<CompoundType>& c, int n)
{
	unsigned   r   = c->getOffsetRemainder(n * 8);
	QString    nam = c->getNameAtOffset(n * 8);
	SharedType t   = c->getTypeAtOffset(n * 8);
	SharedExp  res = Binary::get(opMemberAccess, parent, Const::get(nam));

	assert((r % 8) == 0);

	if (t->resolvesToCompound()) {
		res = accessMember(res, t->as<CompoundType>(), r / 8);
	}
	else if (t->resolvesToPointer() && t->as<PointerType>()->getPointsTo()->resolvesToCompound()) {
		if (r != 0) {
			assert(false);
		}
	}
	else if (t->resolvesToArray()) {
		std::shared_ptr<ArrayType> a = t->as<ArrayType>();
		SharedType                 array_member_type = a->getBaseType();
		int b  = array_member_type->getSize() / 8;
		int br = array_member_type->getSize() % 8;
		assert(br == 0);
		res = Binary::get(opArrayIndex, res, Const::get(n / b));

		if (array_member_type->resolvesToCompound()) {
			res = accessMember(res, array_member_type->as<CompoundType>(), n % b);
		}
	}

	return res;
}


SharedExp convertFromOffsetToCompound(SharedExp parent, std::shared_ptr<CompoundType>& c, unsigned n)
{
	if (n * 8 >= c->getSize()) {
		return nullptr;
	}

	QString nam = c->getNameAtOffset(n * 8);

	if (!nam.isNull() && (nam != "pad")) {
		SharedExp l = Location::memOf(parent);
		return Unary::get(opAddrOf, accessMember(l, c, n));
	}

	return nullptr;
}


SharedExp Binary::polySimplify(bool& bMod)
{
	assert(subExp1 && subExp2);

	SharedExp res = shared_from_this();

	subExp1 = subExp1->polySimplify(bMod);
	subExp2 = subExp2->polySimplify(bMod);

	OPER opSub1 = subExp1->getOper();
	OPER opSub2 = subExp2->getOper();

	if ((opSub1 == opIntConst) && (opSub2 == opIntConst)) {
		// k1 op k2, where k1 and k2 are integer constants
		int  k1     = std::static_pointer_cast<Const>(subExp1)->getInt();
		int  k2     = std::static_pointer_cast<Const>(subExp2)->getInt();
		bool change = true;

		switch (m_oper)
		{
		case opPlus:
			k1 = k1 + k2;
			break;

		case opMinus:
			k1 = k1 - k2;
			break;

		case opDiv:
			k1 = (int)((unsigned)k1 / (unsigned)k2);
			break;

		case opDivs:
			k1 = k1 / k2;
			break;

		case opMod:
			k1 = (int)((unsigned)k1 % (unsigned)k2);
			break;

		case opMods:
			k1 = k1 % k2;
			break;

		case opMult:
			k1 = (int)((unsigned)k1 * (unsigned)k2);
			break;

		case opMults:
			k1 = k1 * k2;
			break;

		case opShiftL:

			if (k2 >= 32) {
				k1 = 0;
			}
			else {
				k1 = k1 << k2;
			}

			break;

		case opShiftR:
			k1 = k1 >> k2;
			break;

		case opShiftRA:
			k1 = (k1 >> k2) | (((1 << k2) - 1) << (32 - k2));
			break;

		case opBitOr:
			k1 = k1 | k2;
			break;

		case opBitAnd:
			k1 = k1 & k2;
			break;

		case opBitXor:
			k1 = k1 ^ k2;
			break;

		case opEquals:
			k1 = (k1 == k2);
			break;

		case opNotEqual:
			k1 = (k1 != k2);
			break;

		case opLess:
			k1 = (k1 < k2);
			break;

		case opGtr:
			k1 = (k1 > k2);
			break;

		case opLessEq:
			k1 = (k1 <= k2);
			break;

		case opGtrEq:
			k1 = (k1 >= k2);
			break;

		case opLessUns:
			k1 = ((unsigned)k1 < (unsigned)k2);
			break;

		case opGtrUns:
			k1 = ((unsigned)k1 > (unsigned)k2);
			break;

		case opLessEqUns:
			k1 = ((unsigned)k1 <= (unsigned)k2);
			break;

		case opGtrEqUns:
			k1 = ((unsigned)k1 >= (unsigned)k2);
			break;

		default:
			change = false;
		}

		if (change) {
			res  = Const::get(k1);
			bMod = true;
			return res;
		}
	}

	if (((m_oper == opBitXor) || (m_oper == opMinus)) && (*subExp1 == *subExp2)) {
		// x ^ x or x - x: result is zero
		res  = Const::get(0);
		bMod = true;
		return res;
	}

	if (((m_oper == opBitOr) || (m_oper == opBitAnd)) && (*subExp1 == *subExp2)) {
		// x | x or x & x: result is x
		res  = subExp1;
		bMod = true;
		return res;
	}

	if ((m_oper == opEquals) && (*subExp1 == *subExp2)) {
		// x == x: result is true
		// delete this;
		res  = std::make_shared<Terminal>(opTrue);
		bMod = true;
		return res;
	}

	// Might want to commute to put an integer constant on the RHS
	// Later simplifications can rely on this (ADD other ops as necessary)
	if ((opSub1 == opIntConst) && ((m_oper == opPlus) || (m_oper == opMult) || (m_oper == opMults) || (m_oper == opBitOr) || (m_oper == opBitAnd))) {
		commute();
		// Swap opSub1 and opSub2 as well
		std::swap(opSub1, opSub2);
		// This is not counted as a modification
	}

	// Similarly for boolean constants
	if (subExp1->isBoolConst() && !subExp2->isBoolConst() && ((m_oper == opAnd) || (m_oper == opOr))) {
		commute();
		// Swap opSub1 and opSub2 as well
		std::swap(opSub1, opSub2);
		// This is not counted as a modification
	}

	// Similarly for adding stuff to the addresses of globals
	if (subExp2->isAddrOf() && subExp2->getSubExp1()->isSubscript() &&
		subExp2->getSubExp1()->getSubExp1()->isGlobal() && (m_oper == opPlus)) {
		commute();
		// Swap opSub1 and opSub2 as well
		std::swap(opSub1, opSub2);
		// This is not counted as a modification
	}

	// check for (x + a) + b where a and b are constants, becomes x + a+b
	if ((m_oper == opPlus) && (opSub1 == opPlus) && (opSub2 == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
		int n = std::static_pointer_cast<Const>(subExp2)->getInt();
		res = res->getSubExp1();
		std::shared_ptr<Const> c_subexp(std::static_pointer_cast<Const>(res->getSubExp2()));
		c_subexp->setInt(c_subexp->getInt() + n);
		bMod = true;
		return res;
	}

	// check for (x - a) + b where a and b are constants, becomes x + -a+b
	if ((m_oper == opPlus) && (opSub1 == opMinus) && (opSub2 == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
		int n = std::static_pointer_cast<Const>(subExp2)->getInt();
		res = res->getSubExp1();
		res->setOper(opPlus);
		std::shared_ptr<Const> c_subexp(std::static_pointer_cast<Const>(res->getSubExp2()));
		c_subexp->setInt(-c_subexp->getInt() + n);
		bMod = true;
		return res;
	}

	// check for (x * k) - x, becomes x * (k-1)
	// same with +
	if (((m_oper == opMinus) || (m_oper == opPlus)) && ((opSub1 == opMults) || (opSub1 == opMult)) &&
		(*subExp2 == *subExp1->getSubExp1())) {
		res = res->getSubExp1();
		res->setSubExp2(Binary::get(m_oper, res->getSubExp2(), Const::get(1)));
		bMod = true;
		return res;
	}

	// check for x + (x * k), becomes x * (k+1)
	if ((m_oper == opPlus) && ((opSub2 == opMults) || (opSub2 == opMult)) && (*subExp1 == *subExp2->getSubExp1())) {
		res = res->getSubExp2();
		res->setSubExp2(Binary::get(opPlus, res->getSubExp2(), Const::get(1)));
		bMod = true;
		return res;
	}

	// Turn a + -K into a - K (K is int const > 0)
	// Also a - -K into a + K (K is int const > 0)
	// Does not count as a change
	if (((m_oper == opPlus) || (m_oper == opMinus)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() < 0)) {
		std::static_pointer_cast<Const>(subExp2)->setInt(-std::static_pointer_cast<const Const>(subExp2)->getInt());
		m_oper = m_oper == opPlus ? opMinus : opPlus;
	}

	// Check for exp + 0  or  exp - 0  or  exp | 0
	if (((m_oper == opPlus) || (m_oper == opMinus) || (m_oper == opBitOr)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0)) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for exp or false
	if ((m_oper == opOr) && subExp2->isFalse()) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for SharedExp 0  or exp & 0
	if (((m_oper == opMult) || (m_oper == opMults) || (m_oper == opBitAnd)) && (opSub2 == opIntConst) &&
		(std::static_pointer_cast<const Const>(subExp2)->getInt() == 0)) {
		// delete res;
		res  = Const::get(0);
		bMod = true;
		return res;
	}

	// Check for exp and false
	if ((m_oper == opAnd) && subExp2->isFalse()) {
		// delete res;
		res  = Terminal::get(opFalse);
		bMod = true;
		return res;
	}

	// Check for SharedExp 1
	if (((m_oper == opMult) || (m_oper == opMults)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1)) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for SharedExp x / x
	if (((m_oper == opDiv) || (m_oper == opDivs)) && ((opSub1 == opMult) || (opSub1 == opMults)) &&
		(*subExp2 == *subExp1->getSubExp2())) {
		res  = res->getSubExp1();
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for exp / 1, becomes exp
	if (((m_oper == opDiv) || (m_oper == opDivs)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1)) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for exp % 1, becomes 0
	if (((m_oper == opMod) || (m_oper == opMods)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1)) {
		res  = Const::get(0);
		bMod = true;
		return res;
	}

	// Check for SharedExp x % x, becomes 0
	if (((m_oper == opMod) || (m_oper == opMods)) && ((opSub1 == opMult) || (opSub1 == opMults)) &&
		(*subExp2 == *subExp1->getSubExp2())) {
		res  = Const::get(0);
		bMod = true;
		return res;
	}

	// Check for exp AND -1 (bitwise AND)
	if ((m_oper == opBitAnd) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == -1)) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for exp AND TRUE (logical AND)
	if ((m_oper == opAnd) &&
	    // Is the below really needed?
		((((opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() != 0))) || subExp2->isTrue())) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for exp OR TRUE (logical OR)
	if ((m_oper == opOr) && ((((opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() != 0))) || subExp2->isTrue())) {
		// delete res;
		res  = Terminal::get(opTrue);
		bMod = true;
		return res;
	}

	// Check for [exp] << k where k is a positive integer const
	int k;

	if ((m_oper == opShiftL) && (opSub2 == opIntConst) && ((k = std::static_pointer_cast<const Const>(subExp2)->getInt(), ((k >= 0) && (k < 32))))) {
		res->setOper(opMult);
		std::static_pointer_cast<Const>(subExp2)->setInt(1 << k);
		bMod = true;
		return res;
	}

	if ((m_oper == opShiftR) && (opSub2 == opIntConst) && ((k = std::static_pointer_cast<const Const>(subExp2)->getInt(), ((k >= 0) && (k < 32))))) {
		res->setOper(opDiv);
		std::static_pointer_cast<Const>(subExp2)->setInt(1 << k);
		bMod = true;
		return res;
	}

	/*
	 *  // Check for -x compare y, becomes x compare -y
	 *  // doesn't count as a change
	 *  if (    isComparison() &&
	 *                  opSub1 == opNeg) {
	 *          SharedExp e = subExp1;
	 *          subExp1 = e->getSubExp1()->clone();
	 *          ;//delete e;
	 *          subExp2 = Unary::get(opNeg, subExp2);
	 *  }
	 *
	 *  // Check for (x + y) compare 0, becomes x compare -y
	 *  if (    isComparison() &&
	 *                  opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0 &&
	 *                  opSub1 == opPlus) {
	 *          ;//delete subExp2;
	 *          Binary *b = (Binary*)subExp1;
	 *          subExp2 = b->subExp2;
	 *          b->subExp2 = 0;
	 *          subExp1 = b->subExp1;
	 *          b->subExp1 = 0;
	 *          ;//delete b;
	 *          subExp2 = Unary::get(opNeg, subExp2);
	 *          bMod = true;
	 *          return res;
	 *  }
	 */
	// Check for (x == y) == 1, becomes x == y
	if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1) && (opSub1 == opEquals)) {
		auto b = std::static_pointer_cast<Binary>(subExp1);
		subExp2 = std::move(b->subExp2);
		subExp1 = std::move(b->subExp1);
		bMod    = true;
		return res;
	}

	// Check for x + -y == 0, becomes x == y
	if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opPlus) &&
		(subExp1->getSubExp2()->getOper() == opIntConst)) {
		auto b = std::static_pointer_cast<Binary>(subExp1);
		int  n = std::static_pointer_cast<Const>(b->subExp2)->getInt();

		if (n < 0) {
			subExp2 = std::move(b->subExp2);
			std::static_pointer_cast<Const>(subExp2)->setInt(-std::static_pointer_cast<const Const>(subExp2)->getInt());
			subExp1 = std::move(b->subExp1);
			bMod    = true;
			return res;
		}
	}

	// Check for (x == y) == 0, becomes x != y
	if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opEquals)) {
		auto b = std::static_pointer_cast<Binary>(subExp1);
		subExp2 = std::move(b->subExp2);
		subExp1 = std::move(b->subExp1);
		bMod    = true;
		res->setOper(opNotEqual);
		return res;
	}

	// Check for (x == y) != 1, becomes x != y
	if ((m_oper == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1) && (opSub1 == opEquals)) {
		auto b = std::static_pointer_cast<Binary>(subExp1);
		subExp2 = std::move(b->subExp2);
		subExp1 = std::move(b->subExp1);
		bMod    = true;
		res->setOper(opNotEqual);
		return res;
	}

	// Check for (x == y) != 0, becomes x == y
	if ((m_oper == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opEquals)) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// Check for (0 - x) != 0, becomes x != 0
	if ((m_oper == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opMinus) &&
		subExp1->getSubExp1()->isIntConst() && (std::static_pointer_cast<const Const>(subExp1->getSubExp1())->getInt() == 0)) {
		res  = Binary::get(opNotEqual, subExp1->getSubExp2()->clone(), subExp2->clone());
		bMod = true;
		return res;
	}

	// Check for (x > y) == 0, becomes x <= y
	if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opGtr)) {
		auto b = std::static_pointer_cast<Binary>(subExp1);
		subExp2 = std::move(b->subExp2);
		subExp1 = std::move(b->subExp1);
		bMod    = true;
		res->setOper(opLessEq);
		return res;
	}

	// Check for (x >u y) == 0, becomes x <=u y
	if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opGtrUns)) {
		auto b = std::static_pointer_cast<Binary>(subExp1);
		subExp2 = std::move(b->subExp2);
		subExp1 = std::move(b->subExp1);
		bMod    = true;
		res->setOper(opLessEqUns);
		return res;
	}

	auto b1 = std::dynamic_pointer_cast<Binary>(subExp1);
	auto b2 = std::dynamic_pointer_cast<Binary>(subExp2);

	// Check for (x <= y) || (x == y), becomes x <= y
	if ((m_oper == opOr) && (opSub2 == opEquals) &&
		((opSub1 == opGtrEq) || (opSub1 == opLessEq) || (opSub1 == opGtrEqUns) || (opSub1 == opLessEqUns)) &&
		(((*b1->subExp1 == *b2->subExp1) && (*b1->subExp2 == *b2->subExp2)) ||
		 ((*b1->subExp1 == *b2->subExp2) && (*b1->subExp2 == *b2->subExp1)))) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// For (a || b) or (a && b) recurse on a and b
	if ((m_oper == opOr) || (m_oper == opAnd)) {
		subExp1 = subExp1->polySimplify(bMod);
		subExp2 = subExp2->polySimplify(bMod);
		return res;
	}

	// check for (x & x), becomes x
	if ((m_oper == opBitAnd) && (*subExp1 == *subExp2)) {
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	// check for a + a*n, becomes a*(n+1) where n is an int
	if ((m_oper == opPlus) && (opSub2 == opMult) && (*subExp1 == *subExp2->getSubExp1()) &&
		(subExp2->getSubExp2()->getOper() == opIntConst)) {
		res = res->getSubExp2();
		res->access<Const, 2>()->setInt(res->access<Const, 2>()->getInt() + 1);
		bMod = true;
		return res;
	}

	// check for a*n*m, becomes a*(n*m) where n and m are ints
	if ((m_oper == opMult) && (opSub1 == opMult) && (opSub2 == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
		int m = std::static_pointer_cast<const Const>(subExp2)->getInt();
		res = res->getSubExp1();
		res->access<Const, 2>()->setInt(res->access<Const, 2>()->getInt() * m);
		bMod = true;
		return res;
	}

	// check for !(a == b) becomes a != b
	if ((m_oper == opLNot) && (opSub1 == opEquals)) {
		res = res->getSubExp1();
		res->setOper(opNotEqual);
		bMod = true;
		return res;
	}

	// check for !(a != b) becomes a == b
	if ((m_oper == opLNot) && (opSub1 == opNotEqual)) {
		res = res->getSubExp1();
		res->setOper(opEquals);
		bMod = true;
		return res;
	}

#if 0 // FIXME! ADHOC TA assumed!
	// check for (exp + x) + n where exp is a pointer to a compound type becomes (exp + n) + x
	if ((op == opPlus) &&
		(subExp1->getOper() == opPlus) &&
		subExp1->getSubExp1()->getType() &&
		(subExp2->getOper() == opIntConst)) {
		SharedType ty = subExp1->getSubExp1()->getType();

		if (ty->resolvesToPointer() &&
			ty->as<PointerType>()->getPointsTo()->resolvesToCompound()) {
			res  = Binary::get(opPlus, subExp1->getSubExp1(), subExp2);
			res  = Binary::get(opPlus, res, subExp1->getSubExp2());
			bMod = true;
			return res;
		}
	}
#endif

	// FIXME: suspect this was only needed for ADHOC TA
	// check for exp + n where exp is a pointer to a compound type
	// becomes &m[exp].m + r where m is the member at offset n and r is n - the offset to member m
	SharedType ty = nullptr; // Type of subExp1

	if (subExp1->isSubscript()) {
		Instruction *def = std::static_pointer_cast<RefExp>(subExp1)->getDef();

		if (def) {
			ty = def->getTypeFor(subExp1->getSubExp1());
		}
	}

	if ((m_oper == opPlus) && ty && ty->resolvesToPointer() && ty->as<PointerType>()->getPointsTo()->resolvesToCompound() &&
		(opSub2 == opIntConst)) {
		unsigned n = (unsigned)std::static_pointer_cast<const Const>(subExp2)->getInt();
		std::shared_ptr<CompoundType> c = ty->as<PointerType>()->getPointsTo()->as<CompoundType>();
		res = convertFromOffsetToCompound(subExp1, c, n);

		if (res) {
			LOG_VERBOSE(1) << "(trans1) replacing " << shared_from_this() << " with " << res << "\n";
			bMod = true;
			return res;
		}
	}

#if 0 // FIXME: ADHOC TA assumed
	// check for exp + x where exp is a pointer to an array
	// becomes &exp[x / b] + (x % b) where b is the size of the base type in bytes
	if ((op == opPlus) &&
		subExp1->getType()) {
		SharedExp  x  = subExp2;
		SharedExp  l  = subExp1;
		SharedType ty = l->getType();

		if (ty && ty->resolvesToPointer() &&
			ty->as<PointerType>()->getPointsTo()->resolvesToArray()) {
			auto a  = ty->as<PointerType>()->getPointsTo()->as<ArrayType>();
			int  b  = a->getBaseType()->getSize() / 8;
			int  br = a->getBaseType()->getSize() % 8;
			assert(br == 0);

			if ((x->getOper() != opIntConst) || (((Const *)x)->getInt() >= b) || a->getBaseType()->isArray()) {
				res = Binary::get(opPlus,
								  Unary::get(opAddrOf,
											 Binary::get(opArrayIndex,
														 Location::memOf(l->clone()),
														 Binary::get(opDiv, x->clone(), Const::get(b)))),
								  Binary::get(opMod, x->clone(), Const::get(b)));

				if (VERBOSE) {
					LOG << "replacing " << this << " with " << res << "\n";
				}

				if (l->getOper() == opSubscript) {
					RefExp *r = (RefExp *)l;

					if (r->getDef() && r->getDef()->isPhi()) {
						PhiAssign *pa = (PhiAssign *)r->getDef();
						LOG << "argh: " << pa->getStmtAt(1) << "\n";
					}
				}

				bMod = true;
				return res;
			}
		}
	}
#endif

	if ((m_oper == opFMinus) && (subExp1->getOper() == opFltConst) && (std::static_pointer_cast<const Const>(subExp1)->getFlt() == 0.0)) {
		res  = Unary::get(opFNeg, subExp2);
		bMod = true;
		return res;
	}

	if (((m_oper == opPlus) || (m_oper == opMinus)) && ((subExp1->getOper() == opMults) || (subExp1->getOper() == opMult)) &&
		(subExp2->getOper() == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
		int n1 = std::static_pointer_cast<const Const>(subExp2)->getInt();
		int n2 = subExp1->access<Const, 2>()->getInt();

		if (n1 == n2) {
			res = Binary::get(subExp1->getOper(), Binary::get(m_oper, subExp1->getSubExp1()->clone(), Const::get(1)),
							  Const::get(n1));
			bMod = true;
			return res;
		}
	}

	if (((m_oper == opPlus) || (m_oper == opMinus)) && (subExp1->getOper() == opPlus) && (subExp2->getOper() == opIntConst) &&
		((subExp1->getSubExp2()->getOper() == opMults) || (subExp1->getSubExp2()->getOper() == opMult)) &&
		(subExp1->access<Exp, 2, 2>()->getOper() == opIntConst)) {
		int n1 = std::static_pointer_cast<const Const>(subExp2)->getInt();
		int n2 = subExp1->access<Const, 2, 2>()->getInt();

		if (n1 == n2) {
			res = Binary::get(opPlus, subExp1->getSubExp1(),
							  Binary::get(subExp1->getSubExp2()->getOper(),
										  Binary::get(m_oper, subExp1->access<Exp, 2, 1>()->clone(), Const::get(1)),
										  Const::get(n1)));
			bMod = true;
			return res;
		}
	}

	// check for ((x * a) + (y * b)) / c where a, b and c are all integers and a and b divide evenly by c
	// becomes: (x * a/c) + (y * b/c)
	if ((m_oper == opDiv) && (subExp1->getOper() == opPlus) && (subExp2->getOper() == opIntConst) &&
		(subExp1->getSubExp1()->getOper() == opMult) && (subExp1->getSubExp2()->getOper() == opMult) &&
		(subExp1->access<Exp, 1, 2>()->getOper() == opIntConst) &&
		(subExp1->access<Exp, 2, 2>()->getOper() == opIntConst)) {
		int a = subExp1->access<Const, 1, 2>()->getInt();
		int b = subExp1->access<Const, 2, 2>()->getInt();
		int c = std::static_pointer_cast<const Const>(subExp2)->getInt();

		if (((a % c) == 0) && ((b % c) == 0)) {
			res = Binary::get(opPlus, Binary::get(opMult, subExp1->getSubExp1()->getSubExp1(), Const::get(a / c)),
							  Binary::get(opMult, subExp1->access<Exp, 2, 1>(), Const::get(b / c)));
			bMod = true;
			return res;
		}
	}

	// check for ((x * a) + (y * b)) % c where a, b and c are all integers
	// becomes: (y * b) % c if a divides evenly by c
	// becomes: (x * a) % c if b divides evenly by c
	// becomes: 0            if both a and b divide evenly by c
	if ((m_oper == opMod) && (subExp1->getOper() == opPlus) && (subExp2->getOper() == opIntConst) &&
		(subExp1->getSubExp1()->getOper() == opMult) && (subExp1->getSubExp2()->getOper() == opMult) &&
		(subExp1->getSubExp1()->getSubExp2()->getOper() == opIntConst) &&
		(subExp1->getSubExp2()->getSubExp2()->getOper() == opIntConst)) {
		int a = subExp1->access<Const, 1, 2>()->getInt();
		int b = subExp1->access<Const, 2, 2>()->getInt();
		int c = std::static_pointer_cast<const Const>(subExp2)->getInt();

		if (((a % c) == 0) && ((b % c) == 0)) {
			res  = Const::get(0);
			bMod = true;
			return res;
		}

		if ((a % c) == 0) {
			res  = Binary::get(opMod, subExp1->getSubExp2()->clone(), Const::get(c));
			bMod = true;
			return res;
		}

		if ((b % c) == 0) {
			res  = Binary::get(opMod, subExp1->getSubExp1()->clone(), Const::get(c));
			bMod = true;
			return res;
		}
	}

	// Check for 0 - (0 <u exp1) & exp2 => exp2
	if ((m_oper == opBitAnd) && (opSub1 == opMinus)) {
		SharedExp leftOfMinus = subExp1->getSubExp1();

		if (leftOfMinus->isIntConst() && (std::static_pointer_cast<const Const>(leftOfMinus)->getInt() == 0)) {
			SharedExp rightOfMinus = subExp1->getSubExp2();

			if (rightOfMinus->getOper() == opLessUns) {
				SharedExp leftOfLess = rightOfMinus->getSubExp1();

				if (leftOfLess->isIntConst() && (std::static_pointer_cast<const Const>(leftOfLess)->getInt() == 0)) {
					res  = getSubExp2();
					bMod = true;
					return res;
				}
			}
		}
	}

	// Replace opSize(n, loc) with loc and set the type if needed
	if ((m_oper == opSize) && subExp2->isLocation()) {
#if 0   // FIXME: ADHOC TA assumed here
		Location   *loc = (Location *)subExp2;
		unsigned   n    = (unsigned)((Const *)subExp1)->getInt();
		SharedType ty   = loc->getType();

		if (ty == nullptr) {
			loc->setType(SizeType::get(n));
		}
		else if (ty->getSize() != n) {
			ty->setSize(n);
		}
#endif
		res  = res->getSubExp2();
		bMod = true;
		return res;
	}

	return res;
}


SharedExp Ternary::polySimplify(bool& bMod)
{
	SharedExp res = shared_from_this();

	subExp1 = subExp1->polySimplify(bMod);
	subExp2 = subExp2->polySimplify(bMod);
	subExp3 = subExp3->polySimplify(bMod);

	// p ? 1 : 0 -> p
	if ((m_oper == opTern) && (subExp2->getOper() == opIntConst) && (subExp3->getOper() == opIntConst)) {
		auto s2 = std::static_pointer_cast<Const>(subExp2);
		auto s3 = std::static_pointer_cast<Const>(subExp3);

		if ((s2->getInt() == 1) && (s3->getInt() == 0)) {
			res  = getSubExp1();
			bMod = true;
			return res;
		}
	}

	// 1 ? x : y -> x
	if ((m_oper == opTern) && (subExp1->getOper() == opIntConst) && (std::static_pointer_cast<const Const>(subExp1)->getInt() == 1)) {
		res  = this->getSubExp2();
		bMod = true;
		return res;
	}

	// 0 ? x : y -> y
	if ((m_oper == opTern) && (subExp1->getOper() == opIntConst) && (std::static_pointer_cast<const Const>(subExp1)->getInt() == 0)) {
		res  = this->getSubExp3();
		bMod = true;
		return res;
	}

	if (((m_oper == opSgnEx) || (m_oper == opZfill)) && (subExp3->getOper() == opIntConst)) {
		res  = this->getSubExp3();
		bMod = true;
		return res;
	}

	if ((m_oper == opFsize) && (subExp3->getOper() == opItof) && (*subExp1 == *subExp3->getSubExp2()) &&
		(*subExp2 == *subExp3->getSubExp1())) {
		res  = this->getSubExp3();
		bMod = true;
		return res;
	}

	if ((m_oper == opFsize) && (subExp3->getOper() == opFltConst)) {
		res  = this->getSubExp3();
		bMod = true;
		return res;
	}

	if ((m_oper == opItof) && (subExp3->getOper() == opIntConst) && (subExp2->getOper() == opIntConst) &&
		(std::static_pointer_cast<const Const>(subExp2)->getInt() == 32)) {
		unsigned n = std::static_pointer_cast<const Const>(subExp3)->getInt();
		res  = Const::get(*(float *)&n);
		bMod = true;
		return res;
	}

	if ((m_oper == opFsize) && (subExp3->getOper() == opMemOf) && (subExp3->getSubExp1()->getOper() == opIntConst)) {
		ADDRESS  u  = subExp3->access<Const, 1>()->getAddr();
		auto     l  = std::dynamic_pointer_cast<Location>(subExp3);
		UserProc *p = l->getProc();

		if (p) {
			Prog   *prog = p->getProg();
			bool   ok;
			double d = prog->getFloatConstant(u, ok, std::static_pointer_cast<const Const>(subExp1)->getInt());

			if (ok) {
				if (VERBOSE) {
					LOG << "replacing " << subExp3 << " with " << d << " in " << shared_from_this() << "\n";
				}

				subExp3 = Const::get(d);
				bMod    = true;
				return res;
			}
		}
	}

	if ((m_oper == opTruncu) && subExp3->isIntConst()) {
		int          from = std::static_pointer_cast<const Const>(subExp1)->getInt();
		int          to   = std::static_pointer_cast<const Const>(subExp2)->getInt();
		unsigned int val  = std::static_pointer_cast<const Const>(subExp3)->getInt();

		if (from == 32) {
			if (to == 16) {
				res  = Const::get(ADDRESS::g(val & 0xffff));
				bMod = true;
				return res;
			}

			if (to == 8) {
				res  = Const::get(ADDRESS::g(val & 0xff));
				bMod = true;
				return res;
			}
		}
	}

	if ((m_oper == opTruncs) && subExp3->isIntConst()) {
		int from = std::static_pointer_cast<const Const>(subExp1)->getInt();
		int to   = std::static_pointer_cast<const Const>(subExp2)->getInt();
		int val  = std::static_pointer_cast<const Const>(subExp3)->getInt();

		if (from == 32) {
			if (to == 16) {
				res  = Const::get(val & 0xffff);
				bMod = true;
				return res;
			}

			if (to == 8) {
				res  = Const::get(val & 0xff);
				bMod = true;
				return res;
			}
		}
	}

	return res;
}


SharedExp TypedExp::polySimplify(bool& bMod)
{
	SharedExp res = shared_from_this();

	if (subExp1->getOper() == opRegOf) {
		// type cast on a reg of.. hmm.. let's remove this
		res  = res->getSubExp1();
		bMod = true;
		return res;
	}

	subExp1 = subExp1->simplify();
	return res;
}


SharedExp RefExp::polySimplify(bool& bMod)
{
	SharedExp res = shared_from_this();

	SharedExp tmp = subExp1->polySimplify(bMod);

	if (bMod) {
		subExp1 = tmp;
		return res;
	}

	/* This is a nasty hack.  We assume that %DF{0} is 0.  This happens when string instructions are used without first
	 * clearing the direction flag.  By convention, the direction flag is assumed to be clear on entry to a
	 * procedure.
	 */
	if ((subExp1->getOper() == opDF) && (m_def == nullptr)) {
		res  = Const::get(int(0));
		bMod = true;
		return res;
	}

	// another hack, this time for aliasing
	// FIXME: do we really want this now? Pentium specific, and only handles ax/eax (not al or ah)
	if (subExp1->isRegN(0) &&                                                     // r0 (ax)
		m_def && m_def->isAssign() && ((Assign *)m_def)->getLeft()->isRegN(24)) { // r24 (eax)
		res  = std::make_shared<TypedExp>(IntegerType::get(16), RefExp::get(Location::regOf(24), m_def));
		bMod = true;
		return res;
	}

	// Was code here for bypassing phi statements that are now redundant

	return res;
}


SharedExp Unary::simplifyAddr()
{
	SharedExp sub;

	if ((m_oper == opMemOf) && subExp1->isAddrOf()) {
		return getSubExp1()->getSubExp1();
	}

	if (m_oper != opAddrOf) {
		// Not a[ anything ]. Recurse
		subExp1 = subExp1->simplifyAddr();
		return shared_from_this();
	}

	if (subExp1->getOper() == opMemOf) {
		return getSubExp1()->getSubExp1();
	}

	if (subExp1->getOper() == opSize) {
		sub = subExp1->getSubExp2();

		if (sub->getOper() == opMemOf) {
			// Remove the a[
			auto b = getSubExp1();
			// Remove the size[
			auto u = b->getSubExp2();
			// Remove the m[
			return u->getSubExp1();
		}
	}

	// a[ something else ]. Still recurse, just in case
	subExp1 = subExp1->simplifyAddr();
	return shared_from_this();
}


SharedExp Binary::simplifyAddr()
{
	assert(subExp1 && subExp2);

	subExp1 = subExp1->simplifyAddr();
	subExp2 = subExp2->simplifyAddr();
	return shared_from_this();
}


SharedExp Ternary::simplifyAddr()
{
	subExp1 = subExp1->simplifyAddr();
	subExp2 = subExp2->simplifyAddr();
	subExp3 = subExp3->simplifyAddr();
	return shared_from_this();
}


const char *Exp::getOperName() const
{
	return operStrings[m_oper];
}


QString Exp::toString() const
{
	QString     res;
	QTextStream os(&res);

	this->print(os);
	return res;
}


void Exp::printt(QTextStream& os /*= cout*/) const
{
	print(os);

	if (m_oper != opTypedExp) {
		return;
	}

	SharedType t = ((TypedExp *)this)->getType();
	os << "<" << t->getSize();

	/*      switch (t->getType()) {
	 *          case INTEGER:
	 *                  if (t->getSigned())
	 *                                          os << "i";                // Integer
	 *                  else
	 *                                          os << "u"; break;        // Unsigned
	 *          case FLOATP:    os << "f"; break;
	 *          case DATA_ADDRESS: os << "pd"; break;    // Pointer to Data
	 *          case FUNC_ADDRESS: os << "pc"; break;    // Pointer to Code
	 *          case VARARGS:    os << "v"; break;
	 *          case TBOOLEAN:     os << "b"; break;
	 *          case UNKNOWN:    os << "?"; break;
	 *          case TVOID:        break;
	 *  } */
	os << ">";
}


void Exp::printAsHL(QTextStream& os /*= cout*/)
{
	QString     tgt;
	QTextStream ost(&tgt);

	ost << this; // Print to the string stream

	if ((tgt.length() >= 4) && (tgt[1] == '[')) {
		// r[nn]; change to rnn
		tgt.remove(1, 1);             // '['
		tgt.remove(tgt.length() - 1); // ']'
	}

	os << tgt;                        // Print to the output stream
}


/***************************************************************************/ /**
 * \brief Output operator for Exp*
 * \param os output stream to send to
 * \param p ptr to Exp to print to the stream
 * \returns copy of os (for concatenation)
 ******************************************************************************/
QTextStream& operator<<(QTextStream& os, const Exp *p)
{
#if 1
	// Useful for debugging, but can clutter the output
	p->printt(os);
#else
	p->print(os);
#endif
	return os;
}


SharedExp Exp::fixSuccessor()
{
	bool      change;
	SharedExp result;
	UniqExp   search_expression(new Unary(opSuccessor, Location::regOf(Terminal::get(opWild))));

	// Assume only one successor function in any 1 expression
	if (search(*search_expression, result)) {
		// Result has the matching expression, i.e. succ(r[K])
		SharedExp sub1 = result->getSubExp1();
		assert(sub1->getOper() == opRegOf);
		SharedExp sub2 = sub1->getSubExp1();
		assert(sub2->getOper() == opIntConst);
		// result     sub1    sub2
		// succ(      r[   Const K    ])
		// Note: we need to clone the r[K] part, since it will be ;//deleted as
		// part of the searchReplace below
		auto replace = sub1->clone();
		auto c       = replace->access<Const, 1>();
		c->setInt(c->getInt() + 1); // Do the increment
		SharedExp res = searchReplace(*result, replace, change);
		return res;
	}

	return shared_from_this();
}


SharedExp Exp::killFill()
{
	static Ternary srch1(opZfill, Terminal::get(opWild), Terminal::get(opWild), Terminal::get(opWild));
	static Ternary srch2(opSgnEx, Terminal::get(opWild), Terminal::get(opWild), Terminal::get(opWild));
	SharedExp      res = shared_from_this();

	std::list<SharedExp *> result;
	doSearch(srch1, res, result, false);
	doSearch(srch2, res, result, false);

	for (SharedExp *it : result) {
		// Kill the sign extend bits
		*it = (*it)->getSubExp3();
	}

	return res;
}


bool Exp::isTemp() const
{
	if (m_oper == opTemp) {
		return true;
	}

	if (m_oper != opRegOf) {
		return false;
	}

	// Some old code has r[tmpb] instead of just tmpb
	SharedConstExp sub = getSubExp1();
	return sub->m_oper == opTemp;
}


SharedExp Exp::removeSubscripts(bool& allZero)
{
	auto        e = shared_from_this();
	LocationSet locs;

	e->addUsedLocs(locs);
	LocationSet::iterator xx;
	allZero = true;

	for (xx = locs.begin(); xx != locs.end(); xx++) {
		if ((*xx)->getOper() == opSubscript) {
			auto        r1   = std::static_pointer_cast<RefExp>(*xx);
			Instruction *def = r1->getDef();

			if (!((def == nullptr) || (def->getNumber() == 0))) {
				allZero = false;
			}

			bool change;
			e = e->searchReplaceAll(**xx, r1->getSubExp1() /*->clone()*/,
									change); // TODO: what happens when clone is restored here ?
		}
	}

	return e;
}


SharedExp Exp::fromSSAleft(UserProc *proc, Instruction *d)
{
	auto r = RefExp::get(shared_from_this(), d); // "Wrap" in a ref

	return r->accept(new ExpSsaXformer(proc));
}


// A helper class for comparing Exp*'s sensibly
bool lessExpStar::operator()(const SharedConstExp& x, const SharedConstExp& y) const
{
	return(*x < *y);  // Compare the actual Exps
}


bool lessTI::operator()(const SharedExp& x, const SharedExp& y) const
{
	return(*x << *y);  // Compare the actual Exps
}


//    //    //    //    //    //
//    genConstraints    //
//    //    //    //    //    //

SharedExp Exp::genConstraints(SharedExp /*result*/)
{
	// Default case, no constraints -> return true
	return Terminal::get(opTrue);
}


SharedExp Const::genConstraints(SharedExp result)
{
	if (result->isTypeVal()) {
		// result is a constant type, or possibly a partial type such as ptr(alpha)
		SharedType t     = result->access<TypeVal>()->getType();
		bool       match = false;

		switch (m_oper)
		{
		case opLongConst:
		// An integer constant is compatible with any size of integer, as long is it is in the right range
		// (no test yet) FIXME: is there an endianness issue here?
		case opIntConst:
			match = t->isInteger();

			// An integer constant can also match a pointer to something.  Assume values less than 0x100 can't be a
			// pointer
			if ((unsigned)u.i >= 0x100) {
				match |= t->isPointer();
			}

			// We can co-erce 32 bit constants to floats
			match |= t->isFloat();
			break;

		case opStrConst:

			if (t->isPointer()) {
				auto ptr_type = std::static_pointer_cast<PointerType>(t);
				match = ptr_type->getPointsTo()->isChar() ||
						(ptr_type->getPointsTo()->isArray() &&
						 std::static_pointer_cast<ArrayType>((ptr_type)->getPointsTo())->getBaseType()->isChar());
			}

			break;

		case opFltConst:
			match = t->isFloat();
			break;

		default:
			break;
		}

		if (match) {
			// This constant may require a cast or a change of format. So we generate a constraint.
			// Don't clone 'this', so it can be co-erced after type analysis
			return Binary::get(opEquals, Unary::get(opTypeOf, shared_from_this()), result->clone());
		}
		else {
			// Doesn't match
			return Terminal::get(opFalse);
		}
	}

	// result is a type variable, which is constrained by this constant
	SharedType t;

	switch (m_oper)
	{
	case opIntConst:
		{
			// We have something like local1 = 1234.  Either they are both integer, or both pointer
			SharedType intt = IntegerType::get(0);
			SharedType alph = PointerType::newPtrAlpha();
			return Binary::get(
				opOr, Binary::get(
					opAnd, Binary::get(opEquals, result->clone(), TypeVal::get(intt)),
					Binary::get(opEquals,
								Unary::get(opTypeOf,
					                       // Note: don't clone 'this', so we can change the Const after type analysis!
										   shared_from_this()),
								TypeVal::get(intt))),
				Binary::get(opAnd, Binary::get(opEquals, result->clone(), TypeVal::get(alph)),
							Binary::get(opEquals, Unary::get(opTypeOf, shared_from_this()), TypeVal::get(alph))));
		}

	case opLongConst:
		t = IntegerType::get(64);
		break;

	case opStrConst:
		t = PointerType::get(CharType::get());
		break;

	case opFltConst:
		t = FloatType::get(64); // size is not known. Assume double for now
		break;

	default:
		return nullptr;
	}

	auto      tv = TypeVal::get(t);
	SharedExp e  = Binary::get(opEquals, result->clone(), tv);
	return e;
}


SharedExp Unary::genConstraints(SharedExp result)
{
	if (result->isTypeVal()) {
		// TODO: need to check for conflicts
		return Terminal::get(opTrue);
	}

	switch (m_oper)
	{
	case opRegOf:
	case opParam: // Should be no params at constraint time
	case opGlobal:
	case opLocal:
		return Binary::get(opEquals, Unary::get(opTypeOf, this->clone()), result->clone());

	default:
		break;
	}

	return Terminal::get(opTrue);
}


SharedExp Ternary::genConstraints(SharedExp result)
{
	SharedType argHasToBe = nullptr;
	SharedType retHasToBe = nullptr;

	switch (m_oper)
	{
	case opFsize:
	case opItof:
	case opFtoi:
	case opSgnEx:
		{
			assert(subExp1->isIntConst());
			assert(subExp2->isIntConst());
			int fromSize = std::static_pointer_cast<const Const>(subExp1)->getInt();
			int toSize   = std::static_pointer_cast<const Const>(subExp2)->getInt();

			// Fall through
			switch (m_oper)
			{
			case opFsize:
				argHasToBe = FloatType::get(fromSize);
				retHasToBe = FloatType::get(toSize);
				break;

			case opItof:
				argHasToBe = IntegerType::get(fromSize);
				retHasToBe = FloatType::get(toSize);
				break;

			case opFtoi:
				argHasToBe = FloatType::get(fromSize);
				retHasToBe = IntegerType::get(toSize);
				break;

			case opSgnEx:
				argHasToBe = IntegerType::get(fromSize);
				retHasToBe = IntegerType::get(toSize);
				break;

			default:
				break;
			}
		}

	default:
		break;
	}

	SharedExp res = nullptr;

	if (retHasToBe) {
		if (result->isTypeVal()) {
			// result is a constant type, or possibly a partial type such as
			// ptr(alpha)
			SharedType t = result->access<TypeVal>()->getType();

			// Compare broad types
			if (!(*retHasToBe *= *t)) {
				return Terminal::get(opFalse);
			}

			// else just constrain the arg
		}
		else {
			// result is a type variable, constrained by this Ternary
			res = Binary::get(opEquals, result, TypeVal::get(retHasToBe));
		}
	}

	if (argHasToBe) {
		// Constrain the argument
		SharedExp con = subExp3->genConstraints(TypeVal::get(argHasToBe));

		if (res) {
			res = Binary::get(opAnd, res, con);
		}
		else {
			res = con;
		}
	}

	if (res == nullptr) {
		return Terminal::get(opTrue);
	}

	return res;
}


SharedExp RefExp::genConstraints(SharedExp result)
{
	OPER subOp = subExp1->getOper();

	switch (subOp)
	{
	case opRegOf:
	case opParam:
	case opGlobal:
	case opLocal:
		return Binary::get(opEquals, Unary::get(opTypeOf, this->clone()), result->clone());

	default:
		break;
	}

	return Terminal::get(opTrue);
}


SharedExp Binary::constrainSub(const std::shared_ptr<TypeVal>& typeVal1, const std::shared_ptr<TypeVal>& typeVal2)
{
	assert(subExp1 && subExp2);

	SharedExp con1 = subExp1->genConstraints(typeVal1);
	SharedExp con2 = subExp2->genConstraints(typeVal2);
	return Binary::get(opAnd, con1, con2);
}


SharedExp Binary::genConstraints(SharedExp result)
{
	assert(subExp1 && subExp2);

	SharedType restrictTo = nullptr;

	if (result->isTypeVal()) {
		restrictTo = result->access<TypeVal>()->getType();
	}

	SharedExp res     = nullptr;
	auto      intType = IntegerType::get(0); // Wild size (=0)
	auto      intVal  = TypeVal::get(intType);

	switch (m_oper)
	{
	case opFPlus:
	case opFMinus:
	case opFMult:
	case opFDiv:
		{
			if (restrictTo && !restrictTo->isFloat()) {
				// Result can only be float
				return Terminal::get(opFalse);
			}

			// MVE: what about sizes?
			auto ft  = FloatType::get();
			auto ftv = TypeVal::get(ft);
			res = constrainSub(ftv, ftv);

			if (!restrictTo) {
				// Also constrain the result
				res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), ftv));
			}

			return res;
		}

	case opBitAnd:
	case opBitOr:
	case opBitXor:
		{
			if (restrictTo && !restrictTo->isInteger()) {
				// Result can only be integer
				return Terminal::get(opFalse);
			}

			// MVE: What about sizes?
			auto it  = IntegerType::get(STD_SIZE, 0);
			auto itv = TypeVal::get(it);
			res = constrainSub(itv, itv);

			if (!restrictTo) {
				// Also constrain the result
				res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), itv));
			}

			return res;
		}

	case opPlus:
		{
			// A pointer to anything
			SharedType ptrType = PointerType::newPtrAlpha();
			auto       ptrVal  = TypeVal::get(ptrType); // Type value of ptr to anything

			if (!restrictTo || restrictTo->isInteger()) {
				// int + int -> int
				res = constrainSub(intVal, intVal);

				if (!restrictTo) {
					res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), intVal->clone()));
				}
			}

			if (!restrictTo || restrictTo->isPointer()) {
				// ptr + int -> ptr
				SharedExp res2 = constrainSub(ptrVal, intVal);

				if (!restrictTo) {
					res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal->clone()));
				}

				if (res) {
					res = Binary::get(opOr, res, res2);
				}
				else {
					res = res2;
				}

				// int + ptr -> ptr
				res2 = constrainSub(intVal, ptrVal);

				if (!restrictTo) {
					res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal->clone()));
				}

				if (res) {
					res = Binary::get(opOr, res, res2);
				}
				else {
					res = res2;
				}
			}

			if (res) {
				return res->simplify();
			}

			return Terminal::get(opFalse);
		}

	case opMinus:
		{
			SharedType ptrType = PointerType::newPtrAlpha();
			auto       ptrVal  = TypeVal::get(ptrType);

			if (!restrictTo || restrictTo->isInteger()) {
				// int - int -> int
				res = constrainSub(intVal, intVal);

				if (!restrictTo) {
					res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), intVal->clone()));
				}

				// ptr - ptr -> int
				SharedExp res2 = constrainSub(ptrVal, ptrVal);

				if (!restrictTo) {
					res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), intVal->clone()));
				}

				if (res) {
					res = Binary::get(opOr, res, res2);
				}
				else {
					res = res2;
				}
			}

			if (!restrictTo || restrictTo->isPointer()) {
				// ptr - int -> ptr
				SharedExp res2 = constrainSub(ptrVal, intVal);

				if (!restrictTo) {
					res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal->clone()));
				}

				if (res) {
					res = Binary::get(opOr, res, res2);
				}
				else {
					res = res2;
				}
			}

			if (res) {
				return res->simplify();
			}

			return Terminal::get(opFalse);
		}

	case opSize:
		{
			// This used to be considered obsolete, but now, it is used to carry the size of memOf's from the decoder to
			// here
			assert(subExp1->isIntConst());
			int sz = std::static_pointer_cast<const Const>(subExp1)->getInt();

			if (restrictTo) {
				int rsz = restrictTo->getSize();

				if (rsz == 0) {
					// This is now restricted to the current restrictTo, but
					// with a known size
					SharedType it = restrictTo->clone();
					it->setSize(sz);
					return Binary::get(opEquals, Unary::get(opTypeOf, subExp2), TypeVal::get(it));
				}

				return Terminal::get((rsz == sz) ? opTrue : opFalse);
			}

			// We constrain the size but not the basic type
			return Binary::get(opEquals, result->clone(), TypeVal::get(SizeType::get(sz)));
		}

	default:
		break;
	}

	return Terminal::get(opTrue);
}


SharedExp Location::polySimplify(bool& bMod)
{
	SharedExp res = Unary::polySimplify(bMod);

	if ((res->getOper() == opMemOf) && (res->getSubExp1()->getOper() == opAddrOf)) {
		if (VERBOSE) {
			LOG << "polySimplify " << res << "\n";
		}

		res  = res->getSubExp1()->getSubExp1();
		bMod = true;
		return res;
	}

	// check for m[a[loc.x]] becomes loc.x
	if ((res->getOper() == opMemOf) && (res->getSubExp1()->getOper() == opAddrOf) &&
		(res->getSubExp1()->getSubExp1()->getOper() == opMemberAccess)) {
		res  = subExp1->getSubExp1();
		bMod = true;
		return res;
	}

	return res;
}


void Location::getDefinitions(LocationSet& defs)
{
	// This is a hack to fix aliasing (replace with something general)
	// FIXME! This is x86 specific too. Use -O for overlapped registers!
	if ((m_oper == opRegOf) && (std::static_pointer_cast<const Const>(subExp1)->getInt() == 24)) {
		defs.insert(Location::regOf(0));
	}
}


QString Const::getFuncName() const
{
	return u.pp->getName();
}


SharedExp Unary::simplifyConstraint()
{
	subExp1 = subExp1->simplifyConstraint();
	return shared_from_this();
}


SharedExp Binary::simplifyConstraint()
{
	assert(subExp1 && subExp2);

	subExp1 = subExp1->simplifyConstraint();
	subExp2 = subExp2->simplifyConstraint();

	switch (m_oper)
	{
	case opEquals:

		if (subExp1->isTypeVal() && subExp2->isTypeVal()) {
			// FIXME: ADHOC TA assumed
			SharedType t1 = subExp1->access<TypeVal>()->getType();
			SharedType t2 = subExp2->access<TypeVal>()->getType();

			if (!t1->isPointerToAlpha() && !t2->isPointerToAlpha()) {
				if (*t1 == *t2) {
					return Terminal::get(opTrue);
				}
				else {
					return Terminal::get(opFalse);
				}
			}
		}

		break;

	case opOr:
	case opAnd:
	case opNot:
		return simplify();

	default:
		break;
	}

	return shared_from_this();
}


//    //    //    //    //    //    //    //
//                            //
//       V i s i t i n g        //
//                            //
//    //    //    //    //    //    //    //
bool Unary::accept(ExpVisitor *v)
{
	bool override, ret = v->visit(shared_from_base<Unary>(), override);

	if (override) {
		return ret; // Override the rest of the accept logic
	}

	if (ret) {
		ret = subExp1->accept(v);
	}

	return ret;
}


bool Binary::accept(ExpVisitor *v)
{
	assert(subExp1 && subExp2);

	bool override, ret = v->visit(shared_from_base<Binary>(), override);

	if (override) {
		return ret;
	}

	if (ret) {
		ret = subExp1->accept(v);
	}

	if (ret) {
		ret = subExp2->accept(v);
	}

	return ret;
}


bool Ternary::accept(ExpVisitor *v)
{
	bool override, ret = v->visit(shared_from_base<Ternary>(), override);

	if (override) {
		return ret;
	}

	if (ret) {
		ret = subExp1->accept(v);
	}

	if (ret) {
		ret = subExp2->accept(v);
	}

	if (ret) {
		ret = subExp3->accept(v);
	}

	return ret;
}


bool TypedExp::accept(ExpVisitor *v)
{
	bool override, ret = v->visit(shared_from_base<TypedExp>(), override);

	if (override) {
		return ret;
	}

	if (ret) {
		ret = subExp1->accept(v);
	}

	return ret;
}


bool FlagDef::accept(ExpVisitor *v)
{
	bool override, ret = v->visit(shared_from_base<FlagDef>(), override);

	if (override) {
		return ret;
	}

	if (ret) {
		ret = subExp1->accept(v);
	}

	return ret;
}


bool RefExp::accept(ExpVisitor *v)
{
	bool override;
	bool ret = v->visit(shared_from_base<RefExp>(), override);

	if (override) {
		return ret;
	}

	if (ret) {
		ret = subExp1->accept(v);
	}

	return ret;
}


bool Location::accept(ExpVisitor *v)
{
	bool override = false, ret = v->visit(shared_from_base<Location>(), override);

	if (override) {
		return ret;
	}

	if (ret) {
		ret &= subExp1->accept(v);
	}

	return ret;
}


// The following are similar, but don't have children that have to accept visitors
bool Terminal::accept(ExpVisitor *v)
{
	return v->visit(shared_from_base<Terminal>());
}


bool Const::accept(ExpVisitor *v)
{
	return v->visit(shared_from_base<Const>());
}


bool TypeVal::accept(ExpVisitor *v)
{
	return v->visit(shared_from_base<TypeVal>());
}


void Exp::fixLocationProc(UserProc *p)
{
	// All locations are supposed to have a pointer to the enclosing UserProc that they are a location of. Sometimes,
	// you have an arbitrary expression that may not have all its procs set. This function fixes the procs for all
	// Location subexpresssions.
	FixProcVisitor fpv;

	fpv.setProc(p);
	accept(&fpv);
}


// GetProcVisitor class

UserProc *Exp::findProc()
{
	GetProcVisitor gpv;

	accept(&gpv);
	return gpv.getProc();
}


void Exp::setConscripts(int n, bool bClear)
{
	SetConscripts sc(n, bClear);

	accept(&sc);
}


SharedExp Exp::stripSizes()
{
	SizeStripper ss;

	return accept(&ss);
}


SharedExp Unary::accept(ExpModifier *v)
{
	// This Unary will be changed in *either* the pre or the post visit. If it's changed in the preVisit step, then
	// postVisit doesn't care about the type of ret. So let's call it a Unary, and the type system is happy
	bool recur = false;
	auto ret   = std::dynamic_pointer_cast<Unary>(v->preVisit(shared_from_base<Unary>(), recur));

	if (recur) {
		subExp1 = subExp1->accept(v);
	}

	assert(ret);
	return v->postVisit(ret);
}


SharedExp Binary::accept(ExpModifier *v)
{
	assert(subExp1 && subExp2);

	bool      recur;
	SharedExp ret = v->preVisit(shared_from_base<Binary>(), recur);

	if (recur) {
		subExp1 = subExp1->accept(v);
	}

	if (recur) {
		subExp2 = subExp2->accept(v);
	}

	auto bret = std::dynamic_pointer_cast<Binary>(ret);

	if (bret) {
		return v->postVisit(bret);
	}

	auto uret = std::dynamic_pointer_cast<Unary>(ret);

	if (uret) {
		return v->postVisit(uret);
	}

	Q_ASSERT(false);
	return nullptr;
}


SharedExp Ternary::accept(ExpModifier *v)
{
	bool recur;
	auto ret = std::static_pointer_cast<Ternary>(v->preVisit(shared_from_base<Ternary>(), recur));

	if (recur) {
		subExp1 = subExp1->accept(v);
	}

	if (recur) {
		subExp2 = subExp2->accept(v);
	}

	if (recur) {
		subExp3 = subExp3->accept(v);
	}

	assert(std::dynamic_pointer_cast<Ternary>(ret));
	return v->postVisit(ret);
}


SharedExp Location::accept(ExpModifier *v)
{
	// This looks to be the same source code as Unary::accept, but the type of "this" is different, which is all
	// important here!  (it makes a call to a different visitor member function).
	bool      recur;
	SharedExp ret = v->preVisit(shared_from_base<Location>(), recur);

	if (recur) {
		subExp1 = subExp1->accept(v);
	}

	auto loc_ret = std::dynamic_pointer_cast<Location>(ret);

	if (loc_ret) {
		return v->postVisit(loc_ret);
	}

	auto ref_ret = std::dynamic_pointer_cast<RefExp>(ret);

	if (ref_ret) {
		return v->postVisit(ref_ret);
	}

	assert(false);
	return nullptr;
}


SharedExp RefExp::accept(ExpModifier *v)
{
	bool recur;
	auto ret     = v->preVisit(shared_from_base<RefExp>(), recur);
	auto ref_ret = std::dynamic_pointer_cast<RefExp>(ret);

	if (recur) {
		subExp1 = subExp1->accept(v);
	}

	// TODO: handle the case where Exp modifier changed type of Exp, currently just not calling postVisit!
	if (ref_ret) {
		return v->postVisit(ref_ret);
	}

	return ret;
}


SharedExp FlagDef::accept(ExpModifier *v)
{
	bool recur;
	auto ret        = v->preVisit(shared_from_base<FlagDef>(), recur);
	auto flgdef_ret = std::dynamic_pointer_cast<FlagDef>(ret);

	if (recur) {
		subExp1 = subExp1->accept(v);
	}

	assert(flgdef_ret);
	return v->postVisit(flgdef_ret);
}


SharedExp TypedExp::accept(ExpModifier *v)
{
	bool recur;
	auto ret          = v->preVisit(shared_from_base<TypedExp>(), recur);
	auto typedexp_ret = std::dynamic_pointer_cast<TypedExp>(ret);

	if (recur) {
		subExp1 = subExp1->accept(v);
	}

	assert(typedexp_ret);
	return v->postVisit(typedexp_ret);
}


SharedExp Terminal::accept(ExpModifier *v)
{
	// This is important if we need to modify terminals
	SharedExp val      = v->preVisit(shared_from_base<Terminal>());
	auto      term_res = std::dynamic_pointer_cast<Terminal>(val);

	if (term_res) {
		return v->postVisit(term_res);
	}

	auto ref_res = std::dynamic_pointer_cast<RefExp>(val);

	if (ref_res) {
		return v->postVisit(ref_res);
	}

	assert(false);
	return nullptr;
}


SharedExp Const::accept(ExpModifier *v)
{
	auto ret       = v->preVisit(shared_from_base<Const>());
	auto const_ret = std::dynamic_pointer_cast<Const>(ret);

	assert(const_ret);
	return v->postVisit(const_ret);
}


SharedExp TypeVal::accept(ExpModifier *v)
{
	auto ret         = v->preVisit(shared_from_base<TypeVal>());
	auto typeval_ret = std::dynamic_pointer_cast<TypeVal>(ret);

	assert(typeval_ret);
	return v->postVisit(typeval_ret);
}


QTextStream& alignStream(QTextStream& str, int align)
{
	str << qSetFieldWidth(align) << " " << qSetFieldWidth(0);
	return str;
}


void child(const SharedExp& e, int ind)
{
	if (e == nullptr) {
		alignStream(LOG_STREAM(), ind + 4) << "<nullptr>\n";
		LOG_STREAM().flush();
		return;
	}

	e->printx(ind + 4);
}


void Unary::printx(int ind) const
{
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << "\n";
	LOG_STREAM().flush();
	child(subExp1, ind);
}


void Binary::printx(int ind) const
{
	assert(subExp1 && subExp2);
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << "\n";
	LOG_STREAM().flush();
	child(subExp1, ind);
	child(subExp2, ind);
}


void Ternary::printx(int ind) const
{
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << "\n";
	child(subExp1, ind);
	child(subExp2, ind);
	child(subExp3, ind);
}


void Const::printx(int ind) const
{
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << "\n";

	switch (m_oper)
	{
	case opIntConst:
		LOG_STREAM() << u.i;
		break;

	case opStrConst:
		LOG_STREAM() << "\"" << m_string << "\"";
		break;

	case opFltConst:
		LOG_STREAM() << u.d;
		break;

	case opFuncConst:
		LOG_STREAM() << qPrintable(u.pp->getName());
		break;

	default:
		LOG_STREAM() << "?" << (int)m_oper << "?";
	}

	if (m_conscript) {
		LOG_STREAM() << " \\" << m_conscript << "\\";
	}

	LOG_STREAM() << '\n';
	LOG_STREAM().flush();
}


void TypeVal::printx(int ind) const
{
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << " ";
	LOG_STREAM() << val->getCtype() << "\n";
	LOG_STREAM().flush();
}


void TypedExp::printx(int ind) const
{
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << " ";
	LOG_STREAM() << type->getCtype() << "\n";
	LOG_STREAM().flush();
	child(subExp1, ind);
}


void Terminal::printx(int ind) const
{
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << "\n";
	LOG_STREAM().flush();
}


void RefExp::printx(int ind) const
{
	alignStream(LOG_STREAM(), ind) << operStrings[m_oper] << "\n";
	LOG_STREAM() << "{";

	if (m_def == nullptr) {
		LOG_STREAM() << "nullptr";
	}
	else {
		LOG_STREAM() << ADDRESS::host_ptr(m_def) << "=" << m_def->getNumber();
	}

	LOG_STREAM() << "}\n";
	LOG_STREAM().flush();
	child(subExp1, ind);
}


QString Exp::getAnyStrConst()
{
	SharedExp e = shared_from_this();

	if (m_oper == opAddrOf) {
		e = getSubExp1();

		if (e->m_oper == opSubscript) {
			e = e->getSubExp1();
		}

		if (e->m_oper == opMemOf) {
			e = e->getSubExp1();
		}
	}

	if (e->m_oper != opStrConst) {
		return QString::null;
	}

	return std::static_pointer_cast<const Const>(e)->getStr();
}


void Exp::addUsedLocs(LocationSet& used, bool memOnly)
{
	UsedLocsFinder ulf(used, memOnly);

	accept(&ulf);
}


SharedExp Exp::expSubscriptVar(const SharedExp& e, Instruction *def)
{
	ExpSubscripter es(e, def);

	return accept(&es);
}


SharedExp Exp::expSubscriptValNull(const SharedExp& e)
{
	return expSubscriptVar(e, nullptr);
}


SharedExp Exp::expSubscriptAllNull(/*Cfg* cfg*/)
{
	return expSubscriptVar(Terminal::get(opWild), nullptr /* was nullptr, nullptr, cfg */);
}


std::shared_ptr<Location> Location::local(const QString& nam, UserProc *p)
{
	return std::make_shared<Location>(opLocal, Const::get(nam), p);
}


bool RefExp::isImplicitDef() const
{
	return m_def == nullptr || m_def->getKind() == STMT_IMPASSIGN;
}


SharedExp Exp::bypass()
{
	CallBypasser cb(nullptr);

	return accept(&cb);
}


void Exp::bypassComp()
{
	if (m_oper != opMemOf) {
		return;
	}

	setSubExp1(getSubExp1()->bypass());
}


int Exp::getComplexityDepth(UserProc *proc)
{
	ComplexityFinder cf(proc);

	accept(&cf);
	return cf.getDepth();
}


int Exp::getMemDepth()
{
	MemDepthFinder mdf;

	accept(&mdf);
	return mdf.getDepth();
}


SharedExp Exp::propagateAll()
{
	ExpPropagator ep;

	return accept(&ep);
}


SharedExp Exp::propagateAllRpt(bool& changed)
{
	ExpPropagator ep;

	changed = false;
	SharedExp ret = shared_from_this();

	while (true) {
		ep.clearChanged(); // Want to know if changed this *last* accept()
		ret = ret->accept(&ep);

		if (ep.isChanged()) {
			changed = true;
		}
		else {
			break;
		}
	}

	return ret;
}


bool Exp::containsFlags()
{
	FlagsFinder ff;

	accept(&ff);
	return ff.isFound();
}


bool Exp::containsBadMemof(UserProc *proc)
{
	BadMemofFinder bmf(proc);

	accept(&bmf);
	return bmf.isFound();
}


bool Exp::containsMemof(UserProc *proc)
{
	ExpHasMemofTester ehmt(proc);

	accept(&ehmt);
	return ehmt.getResult();
}
