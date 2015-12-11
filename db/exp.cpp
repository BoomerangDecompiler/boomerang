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
#include "types.h"
#include "statement.h"
#include "cfg.h"
#include "exp.h"
#include "register.h"
#include "rtl.h" // E.g. class ParamEntry in decideType()
#include "proc.h"
#include "signature.h"
#include "prog.h"
#include "operstrings.h" // Defines a large array of strings for the createDotFile etc. functions. Needs -I. to find it
#include "util.h"
#include "boomerang.h"
//#include "transformer.h"
#include "visitor.h"
#include "log.h"
#include <QRegularExpression>
#include <iomanip> // For std::setw etc

extern char debug_buffer[]; ///< For prints functions
static int tlstrchr(const QString &str, char ch);

// Derived class constructors

Const::Const(uint32_t i) : Exp(opIntConst), conscript(0), type(VoidType::get()) { u.i = i; }
Const::Const(int i) : Exp(opIntConst), conscript(0), type(VoidType::get()) { u.i = i; }
Const::Const(QWord ll) : Exp(opLongConst), conscript(0), type(VoidType::get()) { u.ll = ll; }
Const::Const(double d) : Exp(opFltConst), conscript(0), type(VoidType::get()) { u.d = d; }
//Const::Const(const char *p) : Exp(opStrConst), conscript(0), type(VoidType::get()) { u.p = p; }
Const::Const(const QString &p) : Exp(opStrConst), conscript(0), type(VoidType::get()) {
    strin = p;
}
Const::Const(Function *p) : Exp(opFuncConst), conscript(0), type(VoidType::get()) { u.pp = p; }
/// \remark This is bad. We need a way of constructing true unsigned constants
Const::Const(ADDRESS a) : Exp(opIntConst), conscript(0), type(VoidType::get()) {
    assert(a.isSourceAddr());
    u.a = a;
}

// Copy constructor
Const::Const(const Const &o) : Exp(o.op) {
    u = o.u;
    conscript = o.conscript;
    type = o.type;
    strin = o.strin;
}

Terminal::Terminal(OPER op) : Exp(op) {}
Terminal::Terminal(const Terminal &o) : Exp(o.op) {} // Copy constructor

Unary::Unary(OPER op) : Exp(op) /*,subExp1(nullptr)*/ {
    // pointer uninitialized to help out finding usages of null pointers ?
    assert(op != opRegOf);
}

Unary::Unary(OPER op, Exp *e) : Exp(op), subExp1(e) { assert(subExp1); }
Unary::Unary(const Unary &o) : Exp(o.op) {
    subExp1 = o.subExp1->clone();
    assert(subExp1);
}

Binary::Binary(OPER op) : Unary(op) {
    // Initialise the 2nd pointer. The first pointer is initialised in the Unary constructor
    // subExp2 = 0;
}
Binary::Binary(OPER op, Exp *e1, Exp *e2) : Unary(op, e1), subExp2(e2) { assert(subExp1 && subExp2); }
Binary::Binary(const Binary &o) : Unary(op) {
    setSubExp1(subExp1->clone());
    subExp2 = o.subExp2->clone();
    assert(subExp1 && subExp2);
}

Ternary::Ternary(OPER op) : Binary(op) { subExp3 = nullptr; }
Ternary::Ternary(OPER op, Exp *e1, Exp *e2, Exp *e3) : Binary(op, e1, e2) {
    subExp3 = e3;
    assert(subExp1 && subExp2 && subExp3);
}
Ternary::Ternary(const Ternary &o) : Binary(o.op) {
    subExp1 = o.subExp1->clone();
    subExp2 = o.subExp2->clone();
    subExp3 = o.subExp3->clone();
    assert(subExp1 && subExp2 && subExp3);
}

TypedExp::TypedExp() : Unary(opTypedExp), type(nullptr) {}
TypedExp::TypedExp(Exp *e1) : Unary(opTypedExp, e1), type(nullptr) {}
TypedExp::TypedExp(SharedType ty, Exp *e1) : Unary(opTypedExp, e1), type(ty) {}
TypedExp::TypedExp(TypedExp &o) : Unary(opTypedExp) {
    subExp1 = o.subExp1->clone();
    type = o.type->clone();
}

FlagDef::FlagDef(Exp *params, SharedRTL rtl) : Unary(opFlagDef, params), rtl(rtl) {}

RefExp::RefExp(Exp *e, Instruction *d) : Unary(opSubscript, e), def(d) { assert(e); }

TypeVal::TypeVal(SharedType ty) : Terminal(opTypeVal), val(ty) {}

/**
 * Create a new Location expression.
 * \param op Should be opRegOf, opMemOf, opLocal, opGlobal, opParam or opTemp.
 * \param exp - child expression
 * \param p - enclosing procedure, if null this constructor will try to find it.
 */
Location::Location(OPER op, Exp *exp, UserProc *p) : Unary(op, exp), proc(p) {
    assert(op == opRegOf || op == opMemOf || op == opLocal || op == opGlobal || op == opParam || op == opTemp);
    if (p == nullptr) {
        // eep.. this almost always causes problems
        Exp *e = exp;
        if (e) {
            bool giveUp = false;
            while (this->proc == nullptr && !giveUp) {
                switch (e->getOper()) {
                case opRegOf:
                case opMemOf:
                case opTemp:
                case opLocal:
                case opGlobal:
                case opParam:
                    this->proc = ((Location *)e)->getProc();
                    giveUp = true;
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

Location::Location(Location &o) : Unary(o.op, o.subExp1->clone()), proc(o.proc) {}

Unary::~Unary() {
    // Remember to ;//delete all children
    if (subExp1 != nullptr) {
        ; // delete subExp1;
    }
}
Binary::~Binary() {
    if (subExp2 != nullptr) {
        ; // delete subExp2;
    }
    // Note that the first pointer is destructed in the Exp1 destructor
}
Ternary::~Ternary() {
    if (subExp3 != nullptr) {
        ; // delete subExp3;
    }
}
FlagDef::~FlagDef() {
    ; // delete rtl;
}
TypeVal::~TypeVal() {
    ; // delete val;
}

/***************************************************************************/ /**
  *
  * \brief  Set requested subexpression; 1 is first
  * \param  e Pointer to subexpression to set
  * \note   If an expression already exists, it is ;//deleted
  ******************************************************************************/
void Unary::setSubExp1(Exp *e) {
    if (subExp1 != nullptr) {
        ; // delete subExp1;
    }
    subExp1 = e;
    assert(subExp1);
}
void Binary::setSubExp2(Exp *e) {
    if (subExp2 != nullptr) {
        ; // delete subExp2;
    }
    subExp2 = e;
    assert(subExp1 && subExp2);
}
void Ternary::setSubExp3(Exp *e) {
    if (subExp3 != nullptr) {
        ; // delete subExp3;
    }
    subExp3 = e;
    assert(subExp1 && subExp2 && subExp3);
}
/***************************************************************************/ /**
  *
  * \brief        Get subexpression
  * \returns            Pointer to the requested subexpression
  ******************************************************************************/
Exp *Unary::getSubExp1() {
    assert(subExp1);
    return subExp1;
}
const Exp *Unary::getSubExp1() const {
    assert(subExp1);
    return subExp1;
}
Exp *&Unary::refSubExp1() {
    assert(subExp1);
    return subExp1;
}
Exp *Binary::getSubExp2() {
    assert(subExp1 && subExp2);
    return subExp2;
}
const Exp *Binary::getSubExp2() const {
    assert(subExp1 && subExp2);
    return subExp2;
}
Exp *&Binary::refSubExp2() {
    assert(subExp1 && subExp2);
    return subExp2;
}
Exp *Ternary::getSubExp3() {
    assert(subExp1 && subExp2 && subExp3);
    return subExp3;
}
const Exp *Ternary::getSubExp3() const {
    assert(subExp1 && subExp2 && subExp3);
    return subExp3;
}
Exp *&Ternary::refSubExp3() {
    assert(subExp1 && subExp2 && subExp3);
    return subExp3;
}

// This to satisfy the compiler (never gets called!)
Exp *dummy;
Exp *&Exp::refSubExp1() { return dummy; }
Exp *&Exp::refSubExp2() { return dummy; }
Exp *&Exp::refSubExp3() { return dummy; }

/***************************************************************************/ /**
  *
  * \brief        Swap the two subexpressions
  *
  ******************************************************************************/
/// Swap the two subexpressions.
void Binary::commute() {
    std::swap(subExp1,subExp2);
    assert(subExp1 && subExp2);
}

/***************************************************************************/ /**
  *
  * \brief        Virtual function to make a clone of myself, i.e. to create
  *                     a new Exp with the same contents as myself, but not sharing
  *                     any memory. Deleting the clone will not affect this object.
  *                     Pointers to subexpressions are not copied, but also cloned.
  * \returns            Pointer to cloned object
  ******************************************************************************/
Exp *Const::clone() const {
    // Note: not actually cloning the Type* type pointer. Probably doesn't matter with GC
    return new Const(*this);
}
Exp *Terminal::clone() const { return new Terminal(*this); }
Exp *Unary::clone() const {
    assert(subExp1);
    Unary *c = new Unary(op);
    c->subExp1 = subExp1->clone();
    return c;
}
Exp *Binary::clone() const {
    assert(subExp1 && subExp2);
    Binary *c = new Binary(op);
    c->subExp1 = subExp1->clone();
    c->subExp2 = subExp2->clone();
    return c;
}

Exp *Ternary::clone() const {
    assert(subExp1 && subExp2 && subExp3);
    Ternary *c = new Ternary(op);
    c->subExp1 = subExp1->clone();
    c->subExp2 = subExp2->clone();
    c->subExp3 = subExp3->clone();
    return c;
}
Exp *TypedExp::clone() const {
    TypedExp *c = new TypedExp(type, subExp1->clone());
    return c;
}
Exp *RefExp::clone() const {
    RefExp *c = new RefExp(subExp1->clone(), def);
    return c;
}

Exp *TypeVal::clone() const {
    TypeVal *c = new TypeVal(val->clone());
    return c;
}

Exp *Location::clone() const {
    Location *c = new Location(op, subExp1->clone(), proc);
    return c;
}

/***************************************************************************/ /**
  *
  * \brief        Virtual function to compare myself for equality with
  *                    another Exp
  * \param  o - Ref to other Exp
  * \returns            True if equal
  ******************************************************************************/
bool Const::operator==(const Exp &o) const {
    // Note: the casts of o to Const& are needed, else op is protected! Duh.
    if (o.getOper() == opWild)
        return true;
    if (o.getOper() == opWildIntConst && op == opIntConst)
        return true;
    if (o.getOper() == opWildStrConst && op == opStrConst)
        return true;
    if (op != o.getOper())
        return false;
    if ((conscript && conscript != ((Const &)o).conscript) || ((Const &)o).conscript)
        return false;
    switch (op) {
    case opIntConst:
        return u.i == ((Const &)o).u.i;
    case opFltConst:
        return u.d == ((Const &)o).u.d;
    case opStrConst:
        return strin == ((Const &)o).strin;
    default:
        LOG << "Operator== invalid operator " << operStrings[op] << "\n";
        assert(0);
    }
    return false;
}
bool Unary::operator==(const Exp &o) const {
    if (o.getOper() == opWild)
        return true;
    if (o.getOper() == opWildRegOf && op == opRegOf)
        return true;
    if (o.getOper() == opWildMemOf && op == opMemOf)
        return true;
    if (o.getOper() == opWildAddrOf && op == opAddrOf)
        return true;
    if (op != o.getOper())
        return false;
    return *subExp1 == *o.getSubExp1();
}
bool Binary::operator==(const Exp &o) const {
    assert(subExp1 && subExp2);
    if (o.getOper() == opWild)
        return true;
    if(nullptr == dynamic_cast<const Binary *>(&o))
        return false;
    if (op != ((Binary &)o).op)
        return false;
    if (!(*subExp1 == *((Binary &)o).getSubExp1()))
        return false;
    return *subExp2 == *((Binary &)o).getSubExp2();
}

bool Ternary::operator==(const Exp &o) const {
    if (o.getOper() == opWild)
        return true;
    if(nullptr == dynamic_cast<const Ternary *>(&o))
        return false;
    if (op != ((Ternary &)o).op)
        return false;
    if (!(*subExp1 == *((Ternary &)o).getSubExp1()))
        return false;
    if (!(*subExp2 == *((Ternary &)o).getSubExp2()))
        return false;
    return *subExp3 == *((Ternary &)o).getSubExp3();
}
bool Terminal::operator==(const Exp &o) const {
    if (op == opWildIntConst)
        return o.getOper() == opIntConst;
    if (op == opWildStrConst)
        return o.getOper() == opStrConst;
    if (op == opWildMemOf)
        return o.getOper() == opMemOf;
    if (op == opWildRegOf)
        return o.getOper() == opRegOf;
    if (op == opWildAddrOf)
        return o.getOper() == opAddrOf;
    return ((op == opWild) || // Wild matches anything
            (o.getOper() == opWild) || (op == o.getOper()));
}
bool TypedExp::operator==(const Exp &o) const {
    if (((TypedExp &)o).op == opWild)
        return true;
    if (((TypedExp &)o).op != opTypedExp)
        return false;
    // This is the strict type version
    if (*type != *((TypedExp &)o).type)
        return false;
    return *((Unary *)this)->getSubExp1() == *((Unary &)o).getSubExp1();
}

bool RefExp::operator==(const Exp &o) const {
    if (o.getOper() == opWild)
        return true;
    if (o.getOper() != opSubscript)
        return false;
    if (!(*subExp1 == *o.getSubExp1()))
        return false;
    // Allow a def of (Statement*)-1 as a wild card
    if ((long)def == -1)
        return true;
    assert(dynamic_cast<const RefExp *>(&o)!=nullptr);
    // Allow a def of nullptr to match a def of an implicit assignment
    if ((long)((RefExp &)o).def == -1)
        return true;
    if (def == nullptr && ((RefExp &)o).isImplicitDef())
        return true;
    if (((RefExp &)o).def == nullptr && def && def->isImplicit())
        return true;
    return def == ((RefExp &)o).def;
}

bool TypeVal::operator==(const Exp &o) const {
    if (((TypeVal &)o).op == opWild)
        return true;
    if (((TypeVal &)o).op != opTypeVal)
        return false;
    return *val == *((TypeVal &)o).val;
}

/***************************************************************************/ /**
  *
  * \brief        Virtual function to compare myself with another Exp
  * \note            The test for a wildcard is only with this object, not the other object (o).
  *                    So when searching and there could be wildcards, use search == *this not *this == search
  * \param        o - Ref to other Exp
  * \returns            True if equal
  ******************************************************************************/
bool Const::operator<(const Exp &o) const {
    if (op < o.getOper())
        return true;
    if (op > o.getOper())
        return false;
    if (conscript) {
        if (conscript < ((Const &)o).conscript)
            return true;
        if (conscript > ((Const &)o).conscript)
            return false;
    } else if (((Const &)o).conscript)
        return true;
    switch (op) {
    case opIntConst:
        return u.i < ((Const &)o).u.i;
    case opFltConst:
        return u.d < ((Const &)o).u.d;
    case opStrConst:
        return strin < ((Const &)o).strin;
    default:
        LOG << "Operator< invalid operator " << operStrings[op] << "\n";
        assert(0);
    }
    return false;
}
bool Terminal::operator<(const Exp &o) const { return (op < o.getOper()); }

bool Unary::operator<(const Exp &o) const {
    if (op < o.getOper())
        return true;
    if (op > o.getOper())
        return false;
    return *subExp1 < *((Unary &)o).getSubExp1();
}

bool Binary::operator<(const Exp &o) const {
    assert(subExp1 && subExp2);
    if (op < o.getOper())
        return true;
    if (op > o.getOper())
        return false;
    if (*subExp1 < *((Binary &)o).getSubExp1())
        return true;
    if (*((Binary &)o).getSubExp1() < *subExp1)
        return false;
    return *subExp2 < *((Binary &)o).getSubExp2();
}

bool Ternary::operator<(const Exp &o) const {
    if (op < o.getOper())
        return true;
    if (op > o.getOper())
        return false;
    if (*subExp1 < *((Ternary &)o).getSubExp1())
        return true;
    if (*((Ternary &)o).getSubExp1() < *subExp1)
        return false;
    if (*subExp2 < *((Ternary &)o).getSubExp2())
        return true;
    if (*((Ternary &)o).getSubExp2() < *subExp2)
        return false;
    return *subExp3 < *((Ternary &)o).getSubExp3();
}

bool TypedExp::operator<<(const Exp &o) const { // Type insensitive
    if (op < o.getOper())
        return true;
    if (op > o.getOper())
        return false;
    return *subExp1 << *((Unary &)o).getSubExp1();
}

bool TypedExp::operator<(const Exp &o) const { // Type sensitive
    if (op < o.getOper())
        return true;
    if (op > o.getOper())
        return false;
    if (*type < *((TypedExp &)o).type)
        return true;
    if (*((TypedExp &)o).type < *type)
        return false;
    return *subExp1 < *((Unary &)o).getSubExp1();
}

bool RefExp::operator<(const Exp &o) const {
    if (opSubscript < o.getOper())
        return true;
    if (opSubscript > o.getOper())
        return false;
    if (*subExp1 < *((Unary &)o).getSubExp1())
        return true;
    if (*((Unary &)o).getSubExp1() < *subExp1)
        return false;
    // Allow a wildcard def to match any
    if (def == (Instruction *)-1)
        return false; // Not less (equal)
    if (((RefExp &)o).def == (Instruction *)-1)
        return false;
    return def < ((RefExp &)o).def;
}

bool TypeVal::operator<(const Exp &o) const {
    if (opTypeVal < o.getOper())
        return true;
    if (opTypeVal > o.getOper())
        return false;
    return *val < *((TypeVal &)o).val;
}

/***************************************************************************/ /**
  *
  * \brief        Virtual function to compare myself for equality with another Exp, *ignoring subscripts*
  * \param        o - Ref to other Exp
  * \returns            True if equal
  ******************************************************************************/
bool Const::operator*=(Exp &o) {
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    return *this == *other;
}
bool Unary::operator*=(Exp &o) {
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    if (other->getOper() == opWild)
        return true;
    if (other->getOper() == opWildRegOf && op == opRegOf)
        return true;
    if (other->getOper() == opWildMemOf && op == opMemOf)
        return true;
    if (other->getOper() == opWildAddrOf && op == opAddrOf)
        return true;
    if (op != other->getOper())
        return false;
    return *subExp1 *= *((Unary *)other)->getSubExp1();
}
bool Binary::operator*=(Exp &o) {
    assert(subExp1 && subExp2);
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    if (other->getOper() == opWild)
        return true;
    if (op != other->getOper())
        return false;
    if (!(*subExp1 *= *other->getSubExp1()))
        return false;
    return *subExp2 *= *other->getSubExp2();
}

bool Ternary::operator*=(Exp &o) {
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    if (other->getOper() == opWild)
        return true;
    if (op != other->getOper())
        return false;
    if (!(*subExp1 *= *other->getSubExp1()))
        return false;
    if (!(*subExp2 *= *other->getSubExp2()))
        return false;
    return *subExp3 *= *other->getSubExp3();
}
bool Terminal::operator*=(Exp &o) {
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    return *this == *other;
}
bool TypedExp::operator*=(Exp &o) {
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    if (other->getOper() == opWild)
        return true;
    if (other->getOper() != opTypedExp)
        return false;
    // This is the strict type version
    if (*type != *((TypedExp *)other)->type)
        return false;
    return *((Unary *)this)->getSubExp1() *= *((Unary *)other)->getSubExp1();
}

bool RefExp::operator*=(Exp &o) {
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    return *subExp1 *= *other;
}

bool TypeVal::operator*=(Exp &o) {
    Exp *other = &o;
    if (o.getOper() == opSubscript)
        other = o.getSubExp1();
    return *this == *other;
}

//    //    //    //
//    Const    //
//    //    //    //
/***************************************************************************/ /**
  *
  * \brief       "Print" in infix notation the expression to a stream.
  *                  Mainly for debugging, or maybe some low level windows
  * \param       os  - Ref to an output stream
  ******************************************************************************/
void Const::print(QTextStream &os, bool /*html*/) const {
    setLexBegin(os.pos());
    switch (op) {
    case opIntConst:
        if (u.i < -1000 || u.i > 1000)
            os << "0x" << QString::number(u.i,16);
        else
            os << u.i;
        break;
    case opLongConst:
        if ((long long)u.ll < -1000LL || (long long)u.ll > 1000LL)
            os << "0x" << QString::number(u.ll,16) << "LL";
        else
            os << u.ll << "LL";
        break;
    case opFltConst:
        char buf[64];
        sprintf(buf, "%.4f", u.d); // FIXME: needs an intelligent printer
        os << buf;
        break;
    case opStrConst:
        os << "\"" << strin << "\"";
        break;
    default:
        LOG << "Const::print invalid operator " << operStrings[op] << "\n";
        assert(0);
    }
    if (conscript)
        os << "\\" << conscript << "\\";
#ifdef DUMP_TYPES
        if(type)
            os << "T("<<type->prints() << ")";
#endif
    setLexEnd(os.pos());
}

void Const::printNoQuotes(QTextStream &os) {
    if (op == opStrConst)
        os << strin;
    else
        print(os);
}

//    //    //    //
//    Binary    //
//    //    //    //
void Binary::printr(QTextStream &os, bool html) const {
    assert(subExp1 && subExp2);
    // The "r" is for recursive: the idea is that we don't want parentheses at the outer level, but a subexpression
    // (recursed from a higher level), we want the parens (at least for standard infix operators)
    switch (op) {
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

void Binary::print(QTextStream &os, bool html) const {
    assert(subExp1 && subExp2);
    Exp *p1 = ((Binary *)this)->getSubExp1();
    Exp *p2 = ((Binary *)this)->getSubExp2();
    // Special cases
    switch (op) {
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
        ((Const *)p1)->printNoQuotes(os);
        os << "( ";
        p2->printr(os, html);
        os << " )";
        return;
    case opExpTable:
    case opNameTable:
        if (op == opExpTable)
            os << "exptable(";
        else
            os << "nametable(";
        os << p1 << ", " << p2 << ")";
        return;

    case opList:
        // Because "," is the lowest precedence operator, we don't need printr here.
        // Also, same as UQBT, so easier to test
        p1->print(os, html);
        if (!p2->isNil())
            os << ", ";
        p2->print(os, html);
        return;

    case opMemberAccess:
        p1->print(os, html);
        os << ".";
        ((Const *)p2)->printNoQuotes(os);
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
    if (p1 == nullptr)
        os << "<nullptr>";
    else
        p1->printr(os, html);
    switch (op) {
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
        if (html)
            os << " &lt; ";
        else
            os << " < ";
        break;
    case opGtr:
        if (html)
            os << " &gt; ";
        else
            os << " > ";
        break;
    case opLessEq:
        if (html)
            os << " &lt;= ";
        else
            os << " <= ";
        break;
    case opGtrEq:
        if (html)
            os << " &gt;= ";
        else
            os << " >= ";
        break;
    case opLessUns:
        if (html)
            os << " &lt;u ";
        else
            os << " <u ";
        break;
    case opGtrUns:
        if (html)
            os << " &gt;u ";
        else
            os << " >u ";
        break;
    case opLessEqUns:
        if (html)
            os << " &lt;u ";
        else
            os << " <=u ";
        break;
    case opGtrEqUns:
        if (html)
            os << " &gt;=u ";
        else
            os << " >=u ";
        break;
    case opUpper:
        os << " GT ";
        break;
    case opLower:
        os << " LT ";
        break;
    case opShiftL:
        if (html)
            os << " &lt;&lt; ";
        else
            os << " << ";
        break;
    case opShiftR:
        if (html)
            os << " &gt;&gt; ";
        else
            os << " >> ";
        break;
    case opShiftRA:
        if (html)
            os << " &gt;&gt;A ";
        else
            os << " >>A ";
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
        LOG << "Binary::print invalid operator " << operStrings[op] << "\n";
        assert(0);
    }

    if (p2 == nullptr)
        os << "<nullptr>";
    else
        p2->printr(os, html);
}

//    //    //    //    //
//     Terminal    //
//    //    //    //    //
void Terminal::print(QTextStream &os, bool /*html*/) const {
    switch (op) {
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
        LOG << "Terminal::print invalid operator " << operStrings[op] << "\n";
        assert(0);
    }
}

//    //    //    //
//     Unary    //
//    //    //    //
void Unary::print(QTextStream &os, bool html) const {
    Exp *p1 = ((Unary *)this)->getSubExp1();
    switch (op) {
    //    //    //    //    //    //    //
    //    x[ subexpression ]    //
    //    //    //    //    //    //    //
    case opRegOf:
        // Make a special case for the very common case of r[intConst]
        if (p1->isIntConst()) {
            os << "r" << ((Const *)p1)->getInt();
#ifdef DUMP_TYPES
        os << "T("<<((Const *)p1)->getType()<<")";
#endif
            break;
        } else if (p1->isTemp()) {
            // Just print the temp {   // balance }s
            p1->print(os, html);
            break;
        }
        [[clang::fallthrough]];
    // Else fall through
    case opMemOf:
    case opAddrOf:
    case opVar:
    case opTypeOf:
    case opKindOf:
        switch (op) {
        case opRegOf:
            os << "r[";
            break; // e.g. r[r2]
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
        if (op == opVar)
            ((Const *)p1)->printNoQuotes(os);
        // Use print, not printr, because this is effectively the top level again (because the [] act as
        // parentheses)
        else {
            p1->print(os, html);
        }
        os << "]";
#ifdef DUMP_TYPES
        os << "T("<<((Const *)p1)->getType()<<")";
#endif
        break;

    //    //    //    //    //    //    //
    //      Unary operators    //
    //    //    //    //    //    //    //

    case opNot:
    case opLNot:
    case opNeg:
    case opFNeg:
        if (op == opNot)
            os << "~";
        else if (op == opLNot)
            os << "L~";
        else if (op == opFNeg)
            os << "~f ";
        else
            os << "-";
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
        switch (op) {
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
            os << "t[";
            ((Const *)p1)->printNoQuotes(os);
            os << "]";
            return;
        }
    // Temp: just print the string, no quotes
    case opGlobal: //[[clang::fallthrough]];
    case opLocal:
    case opParam:
        // Print a more concise form than param["foo"] (just foo)
        ((Const *)p1)->printNoQuotes(os);
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
        LOG << "Unary::print invalid operator " << operStrings[op] << "\n";
        assert(0);
    }
}

//    //    //    //
//    Ternary //
//    //    //    //
void Ternary::printr(QTextStream &os, bool /*html*/) const {
    // The function-like operators don't need parentheses
    switch (op) {
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

void Ternary::print(QTextStream &os, bool html) const {
    Exp *p1 = ((Ternary *)this)->getSubExp1();
    Exp *p2 = ((Ternary *)this)->getSubExp2();
    Exp *p3 = ((Ternary *)this)->getSubExp3();
    switch (op) {
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
        switch (op) {
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
        if (p1)
            p1->print(os, html);
        else
            os << "<nullptr>";
        os << ",";
        if (p2)
            p2->print(os, html);
        else
            os << "<nullptr>";
        os << ",";
        if (p3)
            p3->print(os, html);
        else
            os << "<nullptr>";
        os << ")";
        return;
    default:
        break;
    }
    // Else must be ?: or @ (traditional ternary operators)
    if (p1)
        p1->printr(os, html);
    else
        os << "<nullptr>";
    if (op == opTern) {
        os << " ? ";
        if (p2)
            p2->printr(os, html);
        else
            os << "<nullptr>";
        os << " : "; // Need wide spacing here
        if (p3)
            p3->print(os, html);
        else
            os << "<nullptr>";
    } else if (op == opAt) {
        os << "@";
        if (p2)
            p2->printr(os, html);
        else
            os << "nullptr>";
        os << ":";
        if (p3)
            p3->printr(os, html);
        else
            os << "nullptr>";
    } else {
        LOG << "Ternary::print invalid operator " << operStrings[op] << "\n";
        assert(0);
    }
}

//    //    //    //
// TypedExp //
//    //    //    //
void TypedExp::print(QTextStream &os, bool html) const {
    os << " ";
    type->starPrint(os);
    Exp *p1 = ((Ternary *)this)->getSubExp1();
    p1->print(os, html);
}

//    //    //    //
//    RefExp    //
//    //    //    //
void RefExp::print(QTextStream &os, bool html) const {
    if (subExp1)
        subExp1->print(os, html);
    else
        os << "<nullptr>";
    if (html)
        os << "<sub>";
    else
        os << "{";
    if (def == (Instruction *)-1)
        os << "WILD";
    else if (def) {
        if (html)
            os << "<a href=\"#stmt" << def->getNumber() << "\">";
        def->printNum(os);
        if (html)
            os << "</a>";
    } else
        os << "-"; // So you can tell the difference with {0}
    if (html)
        os << "</sub>";
    else
        os << "}";
}

//    //    //    //
// TypeVal    //
//    //    //    //
void TypeVal::print(QTextStream &os, bool /*html*/) const {
    if (val)
        os << "<" << val->getCtype() << ">";
    else
        os << "<nullptr>";
}

/***************************************************************************/ /**
  *
  * \brief        Print to a static string (for debugging)
  * \returns            Address of the static buffer
  ******************************************************************************/
char *Exp::prints() {
    QString tgt;
    QTextStream ost(&tgt);
    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}

void Exp::dump() {
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
void Exp::createDotFile(const char *name) {
    QFile fl(name);
    if(!fl.open(QFile::WriteOnly)) {
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
void Const::appendDotFile(QTextStream &of) {
    // We define a unique name for each node as "e123456" if the address of "this" == 0x123456
    of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
    of << operStrings[op] << "\\n" << ADDRESS::host_ptr(this) << " | ";
    switch (op) {
    case opIntConst:
        of << u.i;
        break;
    case opFltConst:
        of << u.d;
        break;
    case opStrConst:
        of << "\\\"" << strin << "\\\"";
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
void Terminal::appendDotFile(QTextStream &of) {
    of << "e" << ADDRESS::host_ptr(this) << " [shape=parallelogram,label=\"";
    if (op == opWild)
        // Note: value is -1, so can't index array
        of << "WILD";
    else
        of << operStrings[op];
    of << "\\n" << ADDRESS::host_ptr(this);
    of << "\"];\n";
}

//    //    //    //
//    Unary    //
//    //    //    //
void Unary::appendDotFile(QTextStream &of) {
    // First a node for this Unary object
    of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
    // The (int) cast is to print the address, not the expression!
    of << operStrings[op] << "\\n" << ADDRESS::host_ptr(this) << " | ";
    of << "<p1>";
    of << " }\"];\n";

    // Now recurse to the subexpression.
    subExp1->appendDotFile(of);

    // Finally an edge for the subexpression
    of << "e" << ADDRESS::host_ptr(this) << "->e" << ADDRESS::host_ptr(subExp1) << ";\n";
}

//    //    //    //
//    Binary    //
//    //    //    //
void Binary::appendDotFile(QTextStream &of) {
    // First a node for this Binary object
    of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
    of << operStrings[op] << "\\n" << ADDRESS::host_ptr(this) << " | ";
    of << "{<p1> | <p2>}";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    subExp2->appendDotFile(of);
    // Now an edge for each subexpression
    of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1) << ";\n";
    of << "e" << ADDRESS::host_ptr(this) << ":p2->e" << ADDRESS::host_ptr(subExp2) << ";\n";
}

//    //    //    //
//    Ternary //
//    //    //    //
void Ternary::appendDotFile(QTextStream &of) {
    // First a node for this Ternary object
    of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
    of << operStrings[op] << "\\n0x" << ADDRESS::host_ptr(this) << " | ";
    of << "{<p1> | <p2> | <p3>}";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    subExp2->appendDotFile(of);
    subExp3->appendDotFile(of);
    // Now an edge for each subexpression
    of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1) << ";\n";
    of << "e" << ADDRESS::host_ptr(this) << ":p2->e" << ADDRESS::host_ptr(subExp2) << ";\n";
    of << "e" << ADDRESS::host_ptr(this) << ":p3->e" << ADDRESS::host_ptr(subExp3) << ";\n";
}
//    //    //    //
// TypedExp //
//    //    //    //
void TypedExp::appendDotFile(QTextStream &of) {
    of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
    of << "opTypedExp\\n" << ADDRESS::host_ptr(this) << " | ";
    // Just display the C type for now
    of << type->getCtype() << " | <p1>";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1) << ";\n";
}

//    //    //    //
//    FlagDef //
//    //    //    //
void FlagDef::appendDotFile(QTextStream &of) {
    of << "e" << ADDRESS::host_ptr(this) << " [shape=record,label=\"{";
    of << "opFlagDef \\n" << ADDRESS::host_ptr(this) << "| ";
    // Display the RTL as "RTL <r1> <r2>..." vertically (curly brackets)
    of << "{ RTL ";
    int n = rtl->size();
    for (int i = 0; i < n; i++)
        of << "| <r" << i << "> ";
    of << "} | <p1> }\"];\n";
    subExp1->appendDotFile(of);
    of << "e" << ADDRESS::host_ptr(this) << ":p1->e" << ADDRESS::host_ptr(subExp1) << ";\n";
}

/***************************************************************************/ /**
  *
  * \brief        Returns true if the expression is r[K] where K is int const
  * \returns            True if matches
  ******************************************************************************/
bool Exp::isRegOfK() {
    if (op != opRegOf)
        return false;
    return ((Unary *)this)->getSubExp1()->getOper() == opIntConst;
}
/***************************************************************************/ /**
  *
  * \brief        Returns true if the expression is r[N] where N is the given int const
  * \param N - the specific register to be tested for
  * \returns            True if matches
  ******************************************************************************/
bool Exp::isRegN(int N) const {
    if (op != opRegOf)
        return false;
    const Exp *sub = ((const Unary *)this)->getSubExp1();
    return (sub->getOper() == opIntConst && ((const Const *)sub)->getInt() == N);
}
/***************************************************************************/ /**
  *
  * \brief        Returns true if is %afp, %afp+k, %afp-k, or a[m[<any of these]]
  * \returns            True if found
  ******************************************************************************/
bool Exp::isAfpTerm() {
    Exp *cur = this;
    if (op == opTypedExp)
        cur = ((Unary *)this)->getSubExp1();
    Exp *p;
    if ((cur->getOper() == opAddrOf) && ((p = ((Unary *)cur)->getSubExp1()), p->getOper() == opMemOf))
        cur = ((Unary *)p)->getSubExp1();

    OPER curOp = cur->getOper();
    if (curOp == opAFP)
        return true;
    if ((curOp != opPlus) && (curOp != opMinus))
        return false;
    // cur must be a Binary* now
    OPER subOp1 = ((Binary *)cur)->getSubExp1()->getOper();
    OPER subOp2 = ((Binary *)cur)->getSubExp2()->getOper();
    return ((subOp1 == opAFP) && (subOp2 == opIntConst));
}

/***************************************************************************/ /**
  *
  * \brief        Returns the index for this var, e.g. if v[2], return 2
  * \returns            The index
  ******************************************************************************/
int Exp::getVarIndex() {
    assert(op == opVar);
    Exp *sub = ((Unary *)this)->getSubExp1();
    return ((Const *)sub)->getInt();
}

/***************************************************************************/ /**
  *
  * \brief        Returns a ptr to the guard exression, or 0 if none
  * \returns            Ptr to the guard, or 0
  ******************************************************************************/
Exp *Exp::getGuard() {
    if (op == opGuard)
        return ((Unary *)this)->getSubExp1();
    return nullptr;
}

/***************************************************************************/ /**
  *
  * \brief        Matches this expression to the given patten
  * \param        pattern to match
  * \returns            list of variable bindings, or nullptr if matching fails
  ******************************************************************************/
Exp *Exp::match(Exp *pattern) {
    if (*this == *pattern)
        return new Terminal(opNil);
    if (pattern->getOper() == opVar) {
        return Binary::get(opList, Binary::get(opEquals, pattern->clone(), this->clone()), new Terminal(opNil));
    }
    return nullptr;
}
Exp *Unary::match(Exp *pattern) {
    assert(subExp1);
    if (op == pattern->getOper()) {
        return subExp1->match(pattern->getSubExp1());
    }
    return Exp::match(pattern);
}
Exp *Binary::match(Exp *pattern) {
    assert(subExp1 && subExp2);
    if (op != pattern->getOper())
        return Exp::match(pattern);
    Exp *b_lhs = subExp1->match(pattern->getSubExp1());
    if (b_lhs == nullptr)
        return nullptr;
    Exp *b_rhs = subExp2->match(pattern->getSubExp2());
    if (b_rhs == nullptr)
        return nullptr;
    if (b_lhs->getOper() == opNil) {
        delete b_lhs;
        return b_rhs;
    }
    if (b_rhs->getOper() == opNil) {
        delete b_rhs;
        return b_lhs;
    }
#if 0
    LOG << "got lhs list " << b_lhs << " and rhs list " << b_rhs << "\n";
#endif
    Exp *result = new Terminal(opNil);
    // TODO: verify that adding (l &&) is not violating unwritten validity assertion
    for (Exp *l = b_lhs; l && (l->getOper() != opNil); l = l->getSubExp2())
        for (Exp *r = b_rhs; r && r->getOper() != opNil; r = r->getSubExp2())
            if ((*l->getSubExp1()->getSubExp1() == *r->getSubExp1()->getSubExp1()) &&
                !(*l->getSubExp1()->getSubExp2() == *r->getSubExp1()->getSubExp2())) {
#if 0
                LOG << "disagreement in match: " << l->getSubExp1()->getSubExp2() << " != " <<
                       r->getSubExp1()->getSubExp2() << "\n";
#endif
                return nullptr; // must be agreement between LHS and RHS
            } else
                result = Binary::get(opList, l->getSubExp1()->clone(), result);
    for (Exp *r = b_rhs; r->getOper() != opNil; r = r->getSubExp2())
        result = Binary::get(opList, r->getSubExp1()->clone(), result);
    return result;
}
Exp *RefExp::match(Exp *pattern) {
    Exp *r = Unary::match(pattern);
    //      if (r)
    return r;
    /*      r = subExp1->match(pattern);
        if (r) {
                bool change;
                r = r->searchReplaceAll(subExp1->clone(), this->clone(), change);
                return r;
        }
        return Exp::match(pattern); */
}
#if 0 // Suspect ADHOC TA only
Exp* TypeVal::match(Exp *pattern) {
    if (op == pattern->getOper()) {
        return val->match(pattern->getType());
    }
    return Exp::match(pattern);
}
#endif
static QRegularExpression variableRegexp("[a-zA-Z0-9]+");

// TODO use regexp ?
#define ISVARIABLE_S(x)                                                                                                \
    (strspn((x.c_str()), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == (x).length())
//#define DEBUG_MATCH

int tlstrchr(const QString &str, char ch) {
    static QMap<QChar,QChar> braces { {'[',']'},{'{','}'},{'(',')'} };
    int i=0,e=str.length();
    for(; i<e; ++i) {
        if(str[i].toLatin1()==ch)
            return i;
        if(braces.contains(str[i])) {
            QChar end_brace = braces[str[i]];
            ++i; //from next char
            for(; i<e; ++i)
                if(str[i]==end_brace)
                    break;
        }
    }
    if(i==e)
        return -1;
    return i;
}

/***************************************************************************/ /**
  *
  * \brief        Matches this expression to the given patten
  * \param pattern to match
  * \param bindings a map
  * \returns            true if match, false otherwise
  ******************************************************************************/
bool Exp::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    // most obvious
    QString tgt;
    QTextStream ostr(&tgt);
    print(ostr);
    if (tgt == pattern)
        return true;

    assert((pattern.lastIndexOf(variableRegexp)==0) == ISVARIABLE_S(pattern.toStdString()));
    // alright, is pattern an acceptable variable?
    if (pattern.lastIndexOf(variableRegexp)==0) {
        bindings[pattern] = this;
        return true;
    }
    // no, fail
    return false;
}
bool Unary::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    if (Exp::match(pattern, bindings))
        return true;
#ifdef DEBUG_MATCH
    LOG << "unary::match " << this << " to " << pattern << ".\n";
#endif
    if (op == opAddrOf && pattern.startsWith("a[") && pattern.endsWith(']')) {
        return subExp1->match(pattern.mid(2,pattern.size() - 1), bindings); // eliminate 'a[' and ']'
    }
    return false;
}
bool Binary::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    if (Exp::match(pattern, bindings))
        return true;
#ifdef DEBUG_MATCH
    LOG << "binary::match " << this << " to " << pattern << ".\n";
#endif
    if (op == opMemberAccess && -1!=tlstrchr(pattern, '.')) {
        QString sub1 = pattern;
        int split_point = tlstrchr(sub1, '.');
        QString follow = sub1.right(sub1.length()-split_point);
        sub1 = sub1.left(split_point);
        if (subExp1->match(sub1, bindings)) {
            assert(subExp2->isStrConst());
            if (follow == ((Const *)subExp2)->getStr())
                return true;
            if ((follow.lastIndexOf(variableRegexp)==0)) {
                bindings[follow] = subExp2;
                return true;
            }
        }
    }
    if (op == opArrayIndex) {
        if (!pattern.endsWith(']'))
            return false;
        QString sub1 = pattern;
        QString sub2 = sub1.mid(sub1.lastIndexOf('[')+1);
        if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings))
            return true;
    }
    if (op == opPlus && -1!=tlstrchr(pattern, '+')) {
        int splitpoint = tlstrchr(pattern, '+');
        QString sub1 = pattern.left(splitpoint);
        QString sub2 = pattern.mid(splitpoint+1).trimmed();
        if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings))
            return true;
    }
    if (op == opMinus && -1!=tlstrchr(pattern, '-')) {
        int splitpoint = tlstrchr(pattern, '-');
        QString sub1 = pattern.left(splitpoint);
        QString sub2 = pattern.mid(splitpoint+1).trimmed();
        if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings))
            return true;
    }
    return false;
}
bool Ternary::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    if (Exp::match(pattern, bindings))
        return true;
#ifdef DEBUG_MATCH
    LOG << "ternary::match " << this << " to " << pattern << ".\n";
#endif
    return false;
}
bool RefExp::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    if (Exp::match(pattern, bindings))
        return true;
#ifdef DEBUG_MATCH
    LOG << "refexp::match " << this << " to " << pattern << ".\n";
#endif
    if (pattern.endsWith('}')) {
        if (pattern[pattern.size()-2] == '-' && def == nullptr) {
            return subExp1->match(pattern.left(pattern.size()-3), bindings); // remove {-}
        }
        int end = pattern.lastIndexOf('{');
        if (end!=-1) {
            // "prefix {number ...}" -> number matches first def ?
            if(pattern.midRef(end+1).toInt()==def->getNumber()) {
                // match "prefix"
                return subExp1->match(pattern.left(end-1),bindings);
            }
        }
    }
    return false;
}
bool Const::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    if (Exp::match(pattern, bindings))
        return true;
#ifdef DEBUG_MATCH
    LOG << "const::match " << this << " to " << pattern << ".\n";
#endif
    return false;
}
bool Terminal::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    if (Exp::match(pattern, bindings))
        return true;
#ifdef DEBUG_MATCH
    LOG << "terminal::match " << this << " to " << pattern << ".\n";
#endif
    return false;
}
bool Location::match(const QString &pattern, std::map<QString, Exp *> &bindings) {
    if (Exp::match(pattern, bindings))
        return true;
#ifdef DEBUG_MATCH
    LOG << "location::match " << this << " to " << pattern << ".\n";
#endif
    if (op == opMemOf || op == opRegOf) {
        if (op == opRegOf && !pattern.startsWith("r["))
            return false;
        if (op == opMemOf && !pattern.startsWith("m["))
            return false;
        if (!pattern.endsWith(']'))
            return false;
        return subExp1->match(pattern.mid(2), bindings); // shouldn't this cut the last ']' ??
    }
    return false;
}

/***************************************************************************/ /**
  *
  * \brief   Search for the given subexpression
  * \note    Caller must free the list li after use, but not the Exp objects that they point to
  * \note    If the top level expression matches, li will contain search
  * \note    Now a static function. Searches pSrc, not this
  * \param   search ptr to Exp we are searching for
  * \param   pSrc ref to ptr to Exp to search. Reason is that we can then overwrite that pointer
  *               to effect a replacement. So we need to append &pSrc in the list. Can't append &this!
  * \param   li   list of Exp** where pointers to the matches are found once: true if not all occurrences to be
  *                    found, false for all
  * \param   once if set to true only the first possible replacement will be made
  *
  ******************************************************************************/
void Exp::doSearch(const Exp &search, Exp *&pSrc, std::list<Exp **> &li, bool once) {
    bool compare;
    compare = (search == *pSrc);
    if (compare) {
        li.push_back(&pSrc); // Success
        if (once)
            return; // No more to do
    }
    // Either want to find all occurrences, or did not match at this level
    // Recurse into children, unless a matching opSubscript
    if (!compare || pSrc->op != opSubscript)
        pSrc->doSearchChildren(search, li, once);
}

/***************************************************************************/ /**
  * \fn Exp::doSearchChildren
  * \brief       Search for the given subexpression in all children
  * \note        Virtual function; different implementation for each subclass of Exp
  * \note            Will recurse via doSearch
  * \param       search - ptr to Exp we are searching for
  * \param       li - list of Exp** where pointers to the matches are found
  * \param       once - true if not all occurrences to be found, false for all
  *
  ******************************************************************************/
void Exp::doSearchChildren(const Exp & search, std::list<Exp **> & li, bool once) {
    Q_UNUSED(search);
    Q_UNUSED(li);
    Q_UNUSED(once);
    return; // Const and Terminal do not override this
}
void Unary::doSearchChildren(const Exp &search, std::list<Exp **> &li, bool once) {
    if (op != opInitValueOf) // don't search child
        doSearch(search, subExp1, li, once);
}
void Binary::doSearchChildren(const Exp &search, std::list<Exp **> &li, bool once) {
    assert(subExp1 && subExp2);
    doSearch(search, subExp1, li, once);
    if (once && li.size())
        return;
    doSearch(search, subExp2, li, once);
}
void Ternary::doSearchChildren(const Exp &search, std::list<Exp **> &li, bool once) {
    doSearch(search, subExp1, li, once);
    if (once && li.size())
        return;
    doSearch(search, subExp2, li, once);
    if (once && li.size())
        return;
    doSearch(search, subExp3, li, once);
}

/***************************************************************************/ /**
  *
  * \brief   Search for the given subexpression, and replace if found
  * \note    If the top level expression matches, return val != this
  * \param       search - reference to Exp we are searching for
  * \param       replace - ptr to Exp to replace it with
  * \param       change - ref to boolean, set true if a change made (else cleared)
  * \returns            True if a change made
  ******************************************************************************/
Exp *Exp::searchReplace(const Exp &search, Exp *replace, bool &change) {

    return searchReplaceAll(search, replace, change, true);
}

/***************************************************************************/ /**
  *
  * \brief   Search for the given subexpression, and replace wherever found
  * \note    If the top level expression matches, something other than "this" will be returned
  * \note    It is possible with wildcards that in very unusual circumstances a replacement will be made to
  *              something that is already deleted.
  * \note    Replacements are cloned. Caller to delete search and replace
  * \param   search     reference to Exp we are searching for
  * \param   replace ptr to Exp to replace it with
  * \param   change set true if a change made; cleared otherwise
  * \param   once - if set to true only the first possible replacement will be made
  * \note    \a change is ALWAYS assigned. No need to clear beforehand.
  * \returns the result (often this, but possibly changed)
  ******************************************************************************/
Exp *Exp::searchReplaceAll(const Exp &search, Exp *replace, bool &change, bool once /* = false */) {
    // TODO: consider working on base object, and only in case when we find the search, use clone call to return the
    // new object ?
    if (this == &search) { // WAT ?
        change = true;
        return replace->clone();
    }
    assert(this != &search);
    std::list<Exp **> li;
    Exp *top = this; // top may change; that's why we have to return it
    doSearch(search, top, li, false);
    std::list<Exp **>::iterator it;
    for (it = li.begin(); it != li.end(); it++) {
        Exp **pp = *it;
        // if (*pp) //delete *pp;         // Delete any existing
        *pp = replace->clone(); // Do the replacement
        if (once) {
            change = true;
            return top;
        }
    }
    change = (li.size() != 0);
    return top;
}

/***************************************************************************/ /**
  *
  * \brief        Search this expression for the given subexpression, and if found, return true and return a pointer
  *                      to the matched expression in result (useful when there are wildcards, e.g. search pattern is
  *r[?]
  *                      result is r[2].
  * \param   search     ptr to Exp we are searching for
  * \param   result     ref to ptr to Exp that matched
  * \returns            True if a match was found
  ******************************************************************************/
bool Exp::search(const Exp &search, Exp *&result) {
    std::list<Exp **> li;
    result = nullptr; // In case it fails; don't leave it unassigned
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to re-use the same search routine
    Exp *top = this;
    doSearch(search, top, li, false);
    if (li.size()) {
        result = *li.front();
        return true;
    }
    return false;
}

/***************************************************************************/ /**
  *
  * \brief        Search this expression for the given subexpression, and for each found, return a pointer to the
  *                      matched expression in result
  * \param   search     ptr to Exp we are searching for
  * \param   result  ref to list of Exp that matched
  * \returns            True if a match was found
  ******************************************************************************/
bool Exp::searchAll(const Exp &search, std::list<Exp *> &result) {
    std::list<Exp **> li;
    // result.clear();    // No! Useful when searching for more than one thing
    // (add to the same list)
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to re-use the same search routine
    Exp *pSrc = this;
    doSearch(search, pSrc, li, false);
    std::list<Exp **>::iterator it;
    for (it = li.begin(); it != li.end(); it++) {
        // li is list of Exp**; result is list of Exp*
        result.push_back(**it);
    }
    return li.size() != 0;
}

// These simplifying functions don't really belong in class Exp, but they know too much about how Exps work
// They can't go into util.so, since then util.so and db.so would co-depend on each other for testing at least
/***************************************************************************/ /**
  *
  * \brief        Takes an expression consisting on only + and - operators and partitions its terms into positive
  *                    non-integer fixed terms, negative non-integer fixed terms and integer terms. For example, given:
  *                       %sp + 108 + n - %sp - 92
  *                    the resulting partition will be:
  *                       positives = { %sp, n }
  *                       negatives = { %sp }
  *                       integers     = { 108, -92 }
  * \note            integers is a vector so we can use the accumulate func
  * \note            Expressions are NOT cloned. Therefore, do not delete the expressions in positives or negatives
  * \param positives - the list of positive terms
  * \param negatives - the list of negative terms
  * \param integers - the vector of integer terms
  * \param negate - determines whether or not to negate the whole expression, i.e. we are on the RHS of an opMinus
  *
  ******************************************************************************/
void Exp::partitionTerms(std::list<Exp *> &positives, std::list<Exp *> &negatives, std::vector<int> &integers,
                         bool negate) {
    Exp *p1, *p2;
    switch (op) {
    case opPlus:
        p1 = ((Binary *)this)->getSubExp1();
        p2 = ((Binary *)this)->getSubExp2();
        p1->partitionTerms(positives, negatives, integers, negate);
        p2->partitionTerms(positives, negatives, integers, negate);
        break;
    case opMinus:
        p1 = ((Binary *)this)->getSubExp1();
        p2 = ((Binary *)this)->getSubExp2();
        p1->partitionTerms(positives, negatives, integers, negate);
        p2->partitionTerms(positives, negatives, integers, !negate);
        break;
    case opTypedExp:
        p1 = ((Binary *)this)->getSubExp1();
        p1->partitionTerms(positives, negatives, integers, negate);
        break;
    case opIntConst: {
        int k = ((Const *)this)->getInt();
        if (negate)
            integers.push_back(-k);
        else
            integers.push_back(k);
        break;
    }
    default:
        // These can be any other expression tree
        if (negate)
            negatives.push_back(this);
        else
            positives.push_back(this);
    }
}

/***************************************************************************/ /**
  *
  * \brief        This method simplifies an expression consisting of + and - at the top level. For example,
  *                    (%sp + 100) - (%sp + 92) will be simplified to 8.
  * \note            Any expression can be so simplified
  * \note            User must ;//delete result
  * \returns            Ptr to the simplified expression
  ******************************************************************************/
Exp *Unary::simplifyArith() {
    if (op == opMemOf || op == opRegOf || op == opAddrOf || op == opSubscript) {
        // assume we want to simplify the subexpression
        subExp1 = subExp1->simplifyArith();
    }
    return this; // Else, do nothing
}

Exp *Ternary::simplifyArith() {
    subExp1 = subExp1->simplifyArith();
    subExp2 = subExp2->simplifyArith();
    subExp3 = subExp3->simplifyArith();
    return this;
}

Exp *Binary::simplifyArith() {
    assert(subExp1 && subExp2);
    subExp1 = subExp1->simplifyArith(); // FIXME: does this make sense?
    subExp2 = subExp2->simplifyArith(); // FIXME: ditto
    if ((op != opPlus) && (op != opMinus))
        return this;

    // Partition this expression into positive non-integer terms, negative
    // non-integer terms and integer terms.
    std::list<Exp *> positives;
    std::list<Exp *> negatives;
    std::vector<int> integers;
    partitionTerms(positives, negatives, integers, false);

    // Now reduce these lists by cancelling pairs
    // Note: can't improve this algorithm using multisets, since can't instantiate multisets of type Exp (only Exp*).
    // The Exp* in the multisets would be sorted by address, not by value of the expression.
    // So they would be unsorted, same as lists!
    std::list<Exp *>::iterator pp = positives.begin();
    std::list<Exp *>::iterator nn = negatives.begin();
    while (pp != positives.end()) {
        bool inc = true;
        while (nn != negatives.end()) {
            if (**pp == **nn) {
                // A positive and a negative that are equal; therefore they cancel
                pp = positives.erase(pp); // Erase the pointers, not the Exps
                nn = negatives.erase(nn);
                inc = false; // Don't increment pp now
                break;
            }
            nn++;
        }
        if (pp == positives.end())
            break;
        if (inc)
            pp++;
    }

    // Summarise the set of integers to a single number.
    int sum = std::accumulate(integers.begin(), integers.end(), 0);

    // Now put all these elements back together and return the result
    if (positives.size() == 0) {
        if (negatives.size() == 0) {
            return new Const(sum);
        } else
            // No positives, some negatives. sum - Acc
            return Binary::get(opMinus, new Const(sum), Exp::Accumulate(negatives));
    }
    if (negatives.size() == 0) {
        // Positives + sum
        if (sum==0) {
            // Just positives
            return Exp::Accumulate(positives);
        } else {
            OPER op = opPlus;
            if (sum < 0) {
                op = opMinus;
                sum = -sum;
            }
            return Binary::get(op, Exp::Accumulate(positives), new Const(sum));
        }
    }
    // Some positives, some negatives
    if (sum==0) {
        // positives - negatives
        return Binary::get(opMinus, Exp::Accumulate(positives), Exp::Accumulate(negatives));
    }
    // General case: some positives, some negatives, a sum
    OPER op = opPlus;
    if (sum < 0) {
        op = opMinus; // Return (pos - negs) - sum
        sum = -sum;
    }
    return Binary::get(op, Binary::get(opMinus, Exp::Accumulate(positives), Exp::Accumulate(negatives)),
                       new Const(sum));
}

/***************************************************************************/ /**
  *
  * \brief        This method creates an expression that is the sum of all expressions in a list.
  *                    E.g. given the list <4,r[8],m[14]> the resulting expression is 4+r[8]+m[14].
  * \note            static (non instance) function
  * \note            Exps ARE cloned
  * \param        exprs - a list of expressions
  * \returns            a new Exp with the accumulation
  ******************************************************************************/
Exp *Exp::Accumulate(std::list<Exp *> exprs) {
    int n = exprs.size();
    if (n == 0)
        return new Const(0);
    if (n == 1)
        return exprs.front()->clone();

    Exp *first = exprs.front()->clone();
    exprs.pop_front();
    auto res = Binary::get(opPlus, first, Accumulate(exprs));
    exprs.push_front(first);
    return res;
}

    /***************************************************************************/ /**
      *
      * \brief        Apply various simplifications such as constant folding. Also canonicalise by putting iteger
      *                    constants on the right hand side of sums, adding of negative constants changed to subtracting
      *                    positive constants, etc.  Changes << k to a multiply
      * \note            User must ;//delete result
      * \note            Address simplification (a[ m[ x ]] == x) is done separately
      * \returns            Ptr to the simplified expression
      *
      * This code is so big, so weird and so lame it's not funny.  What this boils down to is the process of
      *unification.
      * We're trying to do it with a simple iterative algorithm, but the algorithm keeps getting more and more complex.
      * Eventually I will replace this with a simple theorem prover and we'll have something powerful, but until then,
      * dont rely on this code to do anything critical. - trent 8/7/2002
      ******************************************************************************/
#define DEBUG_SIMP 0                                                              // Set to 1 to print every change
Exp *Exp::simplify() {
#if DEBUG_SIMP
    Exp *save = clone();
#endif
    bool bMod = false; // True if simplified at this or lower level
    Exp *res = this;
    // res = ExpTransformer::applyAllTo(res, bMod);
    // return res;
    do {
        bMod = false;
        // Exp *before = res->clone();
        res = res->polySimplify(bMod); // Call the polymorphic simplify
                                       /*      if (bMod) {
                                                       LOG << "polySimplify hit: " << before << " to " << res << "\n";
                                                       // polySimplify is now redundant, if you see this in the log you need to update one of the files
                                          in the
                                                       // transformations directory to include a rule for the reported transform.
                                               } */
    } while (bMod);                    // If modified at this (or a lower) level, redo
// The below is still important. E.g. want to canonicalise sums, so we know that a + K + b is the same as a + b + K
// No! This slows everything down, and it's slow enough as it is. Call only where needed:
// res = res->simplifyArith();
#if DEBUG_SIMP
    if (!(*res == *save))
        std::cout << "simplified " << save << "  to  " << res << "\n";
    ; // delete save;
#endif
    return res;
}

/***************************************************************************/ /**
  *
  * \brief        Do the work of simplification
  * \note            User must ;//delete result
  * \note            Address simplification (a[ m[ x ]] == x) is done separately
  * \returns            Ptr to the simplified expression
  ******************************************************************************/
Exp *Unary::polySimplify(bool &bMod) {
    Exp *res = this;
    subExp1 = subExp1->polySimplify(bMod);

    if (op == opNot || op == opLNot) {
        switch (subExp1->getOper()) {
        case opEquals:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opNotEqual);
            bMod = true;
            return res;
        case opNotEqual:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opEquals);
            bMod = true;
            return res;
        case opLess:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opGtrEq);
            bMod = true;
            return res;
        case opLessEq:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opGtr);
            bMod = true;
            return res;
        case opGtr:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opLessEq);
            bMod = true;
            return res;
        case opGtrEq:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opLess);
            bMod = true;
            return res;
        case opLessUns:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opGtrEqUns);
            bMod = true;
            return res;
        case opLessEqUns:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opGtrUns);
            bMod = true;
            return res;
        case opGtrUns:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opLessEqUns);
            bMod = true;
            return res;
        case opGtrEqUns:
            res = ((Unary *)res)->getSubExp1();
            res->setOper(opLessUns);
            bMod = true;
            return res;
        default:
            break;
        }
    }

    switch (op) {
    case opNeg:
    case opNot:
    case opLNot:
    case opSize: {
        OPER subOP = subExp1->getOper();
        if (subOP == opIntConst) {
            // -k, ~k, or !k
            OPER op2 = op;
            res = ((Unary *)res)->getSubExp1();
            int k = ((Const *)res)->getInt();
            switch (op2) {
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
            ((Const *)res)->setInt(k);
            bMod = true;
        } else if (op == subOP) {
            res = ((Unary *)res)->getSubExp1();
            res = ((Unary *)res)->getSubExp1();
            bMod = true;
            break;
        }
    } break;
    case opAddrOf:
        // check for a[m[x]], becomes x
        if (subExp1->getOper() == opMemOf) {
            res = ((Unary *)res)->getSubExp1();
            res = ((Unary *)res)->getSubExp1();
            bMod = true;
            return res;
        }
        break;
    case opMemOf:
    case opRegOf: {
        subExp1 = subExp1->polySimplify(bMod);
        // The below IS bad now. It undoes the simplification of
        // m[r29 + -4] to m[r29 - 4]
        // If really needed, do another polySimplify, or swap the order
        // subExp1 = subExp1->simplifyArith();        // probably bad
    } break;
    default:
        break;
    }

    return res;
}
Exp * accessMember(Exp *parent,const std::shared_ptr<CompoundType> &c,int n) {
    unsigned r = c->getOffsetRemainder(n * 8);
    QString nam = c->getNameAtOffset(n * 8);
    SharedType t = c->getTypeAtOffset(n * 8);
    Exp *res = Binary::get(opMemberAccess, parent, Const::get(nam));
    assert((r % 8) == 0);
    if(t->resolvesToCompound()) {
        res = accessMember(res,t->asCompound(),r/8);
    } else if(t->resolvesToPointer() && t->asPointer()->getPointsTo()->resolvesToCompound()) {
        if(r!=0)
            assert(false);
    } else if(t->resolvesToArray()) {
        std::shared_ptr<ArrayType> a = t->asArray();
        SharedType array_member_type = a->getBaseType();
        int b  = array_member_type->getSize() / 8;
        int br = array_member_type->getSize() % 8;
        assert(br==0);
        res = Binary::get(opArrayIndex, res,Const::get(n/b));
        if(array_member_type->resolvesToCompound()) {
            res = accessMember(res,array_member_type->asCompound(),n%b);
        }

    }
    return res;

}
Exp * convertFromOffsetToCompound(Exp *parent,std::shared_ptr<CompoundType> &c,int n) {
    if (n * 8 >= c->getSize())
        return nullptr;
    QString nam = c->getNameAtOffset(n * 8);
    if ( !nam.isNull() && nam != "pad") {
        Exp *l = Location::memOf(parent);
        return new Unary(opAddrOf, accessMember(l,c,n));
    }
    return nullptr;
}
Exp *Binary::polySimplify(bool &bMod) {
    assert(subExp1 && subExp2);

    Exp *res = this;

    subExp1 = subExp1->polySimplify(bMod);
    subExp2 = subExp2->polySimplify(bMod);

    OPER opSub1 = subExp1->getOper();
    OPER opSub2 = subExp2->getOper();

    if ((opSub1 == opIntConst) && (opSub2 == opIntConst)) {
        // k1 op k2, where k1 and k2 are integer constants
        int k1 = ((Const *)subExp1)->getInt();
        int k2 = ((Const *)subExp2)->getInt();
        bool change = true;
        switch (op) {
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
            if(k2>=32)
                k1 = 0;
            else
                k1 = k1 << k2;
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
            ; // delete res;
            res = new Const(k1);
            bMod = true;
            return res;
        }
    }

    if (((op == opBitXor) || (op == opMinus)) && (*subExp1 == *subExp2)) {
        // x ^ x or x - x: result is zero
        res = new Const(0);
        bMod = true;
        return res;
    }

    if (((op == opBitOr) || (op == opBitAnd)) && (*subExp1 == *subExp2)) {
        // x | x or x & x: result is x
        res = subExp1;
        bMod = true;
        return res;
    }

    if (op == opEquals && *subExp1 == *subExp2) {
        // x == x: result is true
        ; // delete this;
        res = new Terminal(opTrue);
        bMod = true;
        return res;
    }

    // Might want to commute to put an integer constant on the RHS
    // Later simplifications can rely on this (ADD other ops as necessary)
    if (opSub1 == opIntConst && (op == opPlus || op == opMult || op == opMults || op == opBitOr || op == opBitAnd)) {
        commute();
        // Swap opSub1 and opSub2 as well
        OPER t = opSub1;
        opSub1 = opSub2;
        opSub2 = t;
        // This is not counted as a modification
    }

    // Similarly for boolean constants
    if (subExp1->isBoolConst() && !subExp2->isBoolConst() && (op == opAnd || op == opOr)) {
        commute();
        // Swap opSub1 and opSub2 as well
        OPER t = opSub1;
        opSub1 = opSub2;
        opSub2 = t;
        // This is not counted as a modification
    }

    // Similarly for adding stuff to the addresses of globals
    if (subExp2->isAddrOf() && subExp2->getSubExp1()->isSubscript() &&
        subExp2->getSubExp1()->getSubExp1()->isGlobal() && op == opPlus) {
        commute();
        // Swap opSub1 and opSub2 as well
        std::swap(opSub1,opSub2);
        // This is not counted as a modification
    }

    // check for (x + a) + b where a and b are constants, becomes x + a+b
    if (op == opPlus && opSub1 == opPlus && opSub2 == opIntConst && subExp1->getSubExp2()->getOper() == opIntConst) {
        int n = ((Const *)subExp2)->getInt();
        res = ((Binary *)res)->getSubExp1();
        ((Const *)res->getSubExp2())->setInt(((Const *)res->getSubExp2())->getInt() + n);
        bMod = true;
        return res;
    }

    // check for (x - a) + b where a and b are constants, becomes x + -a+b
    if (op == opPlus && opSub1 == opMinus && opSub2 == opIntConst && subExp1->getSubExp2()->getOper() == opIntConst) {
        int n = ((Const *)subExp2)->getInt();
        res = ((Binary *)res)->getSubExp1();
        res->setOper(opPlus);
        ((Const *)res->getSubExp2())->setInt((-((Const *)res->getSubExp2())->getInt()) + n);
        bMod = true;
        return res;
    }

    // check for (x * k) - x, becomes x * (k-1)
    // same with +
    if ((op == opMinus || op == opPlus) && (opSub1 == opMults || opSub1 == opMult) &&
        *subExp2 == *subExp1->getSubExp1()) {
        res = ((Binary *)res)->getSubExp1();
        res->setSubExp2(Binary::get(op, res->getSubExp2(), new Const(1)));
        bMod = true;
        return res;
    }

    // check for x + (x * k), becomes x * (k+1)
    if (op == opPlus && (opSub2 == opMults || opSub2 == opMult) && *subExp1 == *subExp2->getSubExp1()) {
        res = ((Binary *)res)->getSubExp2();
        res->setSubExp2(Binary::get(opPlus, res->getSubExp2(), new Const(1)));
        bMod = true;
        return res;
    }

    // Turn a + -K into a - K (K is int const > 0)
    // Also a - -K into a + K (K is int const > 0)
    // Does not count as a change
    if ((op == opPlus || op == opMinus) && opSub2 == opIntConst && ((Const *)subExp2)->getInt() < 0) {
        ((Const *)subExp2)->setInt(-((Const *)subExp2)->getInt());
        op = op == opPlus ? opMinus : opPlus;
    }

    // Check for exp + 0  or  exp - 0  or  exp | 0
    if ((op == opPlus || op == opMinus || op == opBitOr) && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 0) {
        res = ((Binary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp or false
    if (op == opOr && subExp2->isFalse()) {
        res = ((Binary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp * 0  or exp & 0
    if ((op == opMult || op == opMults || op == opBitAnd) && opSub2 == opIntConst &&
        ((Const *)subExp2)->getInt() == 0) {
        ; // delete res;
        res = new Const(0);
        bMod = true;
        return res;
    }

    // Check for exp and false
    if (op == opAnd && subExp2->isFalse()) {
        ; // delete res;
        res = new Terminal(opFalse);
        bMod = true;
        return res;
    }

    // Check for exp * 1
    if ((op == opMult || op == opMults) && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 1) {
        res = ((Unary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp * x / x
    if ((op == opDiv || op == opDivs) && (opSub1 == opMult || opSub1 == opMults) &&
        *subExp2 == *subExp1->getSubExp2()) {
        res = ((Unary *)res)->getSubExp1();
        res = ((Unary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp / 1, becomes exp
    if ((op == opDiv || op == opDivs) && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 1) {
        res = ((Binary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp % 1, becomes 0
    if ((op == opMod || op == opMods) && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 1) {
        res = new Const(0);
        bMod = true;
        return res;
    }

    // Check for exp * x % x, becomes 0
    if ((op == opMod || op == opMods) && (opSub1 == opMult || opSub1 == opMults) &&
        *subExp2 == *subExp1->getSubExp2()) {
        res = new Const(0);
        bMod = true;
        return res;
    }

    // Check for exp AND -1 (bitwise AND)
    if ((op == opBitAnd) && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == -1) {
        res = ((Unary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp AND TRUE (logical AND)
    if ((op == opAnd) &&
        // Is the below really needed?
        (((opSub2 == opIntConst && ((Const *)subExp2)->getInt() != 0)) || subExp2->isTrue())) {
        res = ((Unary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp OR TRUE (logical OR)
    if ((op == opOr) && (((opSub2 == opIntConst && ((Const *)subExp2)->getInt() != 0)) || subExp2->isTrue())) {
        // delete res;
        res = new Terminal(opTrue);
        bMod = true;
        return res;
    }

    // Check for [exp] << k where k is a positive integer const
    int k;
    if (op == opShiftL && opSub2 == opIntConst && ((k = ((Const *)subExp2)->getInt(), (k >= 0 && k < 32)))) {
        res->setOper(opMult);
        ((Const *)subExp2)->setInt(1 << k);
        bMod = true;
        return res;
    }

    if (op == opShiftR && opSub2 == opIntConst && ((k = ((Const *)subExp2)->getInt(), (k >= 0 && k < 32)))) {
        res->setOper(opDiv);
        ((Const *)subExp2)->setInt(1 << k);
        bMod = true;
        return res;
    }

    /*
        // Check for -x compare y, becomes x compare -y
        // doesn't count as a change
        if (    isComparison() &&
                        opSub1 == opNeg) {
                Exp *e = subExp1;
                subExp1 = e->getSubExp1()->clone();
                ;//delete e;
                subExp2 = new Unary(opNeg, subExp2);
        }

        // Check for (x + y) compare 0, becomes x compare -y
        if (    isComparison() &&
                        opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0 &&
                        opSub1 == opPlus) {
                ;//delete subExp2;
                Binary *b = (Binary*)subExp1;
                subExp2 = b->subExp2;
                b->subExp2 = 0;
                subExp1 = b->subExp1;
                b->subExp1 = 0;
                ;//delete b;
                subExp2 = new Unary(opNeg, subExp2);
                bMod = true;
                return res;
        }
*/
    // Check for (x == y) == 1, becomes x == y
    if (op == opEquals && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 1 && opSub1 == opEquals) {
        ; // delete subExp2;
        Binary *b = (Binary *)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = nullptr;
        subExp1 = b->subExp1;
        b->subExp1 = nullptr;
        ; // delete b;
        bMod = true;
        return res;
    }

    // Check for x + -y == 0, becomes x == y
    if (op == opEquals && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 0 && opSub1 == opPlus &&
        ((Binary *)subExp1)->subExp2->getOper() == opIntConst) {
        Binary *b = (Binary *)subExp1;
        int n = ((Const *)b->subExp2)->getInt();
        if (n < 0) {
            ; // delete subExp2;
            subExp2 = b->subExp2;
            ((Const *)subExp2)->setInt(-((Const *)subExp2)->getInt());
            b->subExp2 = nullptr;
            subExp1 = b->subExp1;
            b->subExp1 = nullptr;
            ; // delete b;
            bMod = true;
            return res;
        }
    }

    // Check for (x == y) == 0, becomes x != y
    if (op == opEquals && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 0 && opSub1 == opEquals) {
        ; // delete subExp2;
        Binary *b = (Binary *)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = nullptr;
        subExp1 = b->subExp1;
        b->subExp1 = nullptr;
        ; // delete b;
        bMod = true;
        res->setOper(opNotEqual);
        return res;
    }

    // Check for (x == y) != 1, becomes x != y
    if (op == opNotEqual && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 1 && opSub1 == opEquals) {
        ; // delete subExp2;
        Binary *b = (Binary *)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = nullptr;
        subExp1 = b->subExp1;
        b->subExp1 = nullptr;
        ; // delete b;
        bMod = true;
        res->setOper(opNotEqual);
        return res;
    }

    // Check for (x == y) != 0, becomes x == y
    if (op == opNotEqual && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 0 && opSub1 == opEquals) {
        res = ((Binary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // Check for (0 - x) != 0, becomes x != 0
    if (op == opNotEqual && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 0 && opSub1 == opMinus &&
        subExp1->getSubExp1()->isIntConst() && ((Const *)subExp1->getSubExp1())->getInt() == 0) {
        res = Binary::get(opNotEqual, subExp1->getSubExp2()->clone(), subExp2->clone());
        bMod = true;
        return res;
    }

    // Check for (x > y) == 0, becomes x <= y
    if (op == opEquals && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 0 && opSub1 == opGtr) {
        ; // delete subExp2;
        Binary *b = (Binary *)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = nullptr;
        subExp1 = b->subExp1;
        b->subExp1 = nullptr;
        ; // delete b;
        bMod = true;
        res->setOper(opLessEq);
        return res;
    }

    // Check for (x >u y) == 0, becomes x <=u y
    if (op == opEquals && opSub2 == opIntConst && ((Const *)subExp2)->getInt() == 0 && opSub1 == opGtrUns) {
        ; // delete subExp2;
        Binary *b = (Binary *)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = nullptr;
        subExp1 = b->subExp1;
        b->subExp1 = nullptr;
        ; // delete b;
        bMod = true;
        res->setOper(opLessEqUns);
        return res;
    }

    Binary *b1 = dynamic_cast<Binary *>(subExp1);
    Binary *b2 = dynamic_cast<Binary *>(subExp2);
    // Check for (x <= y) || (x == y), becomes x <= y
    if (op == opOr && opSub2 == opEquals &&
        (opSub1 == opGtrEq || opSub1 == opLessEq || opSub1 == opGtrEqUns || opSub1 == opLessEqUns) &&
        ((*b1->subExp1 == *b2->subExp1 && *b1->subExp2 == *b2->subExp2) ||
         (*b1->subExp1 == *b2->subExp2 && *b1->subExp2 == *b2->subExp1))) {
        res = ((Binary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // For (a || b) or (a && b) recurse on a and b
    if (op == opOr || op == opAnd) {
        subExp1 = subExp1->polySimplify(bMod);
        subExp2 = subExp2->polySimplify(bMod);
        return res;
    }

    // check for (x & x), becomes x
    if (op == opBitAnd && *subExp1 == *subExp2) {
        res = ((Binary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    // check for a + a*n, becomes a*(n+1) where n is an int
    if (op == opPlus && opSub2 == opMult && *subExp1 == *subExp2->getSubExp1() &&
        subExp2->getSubExp2()->getOper() == opIntConst) {
        res = ((Binary *)res)->getSubExp2();
        ((Const *)res->getSubExp2())->setInt(((Const *)res->getSubExp2())->getInt() + 1);
        bMod = true;
        return res;
    }

    // check for a*n*m, becomes a*(n*m) where n and m are ints
    if (op == opMult && opSub1 == opMult && opSub2 == opIntConst && subExp1->getSubExp2()->getOper() == opIntConst) {
        int m = ((Const *)subExp2)->getInt();
        res = ((Binary *)res)->getSubExp1();
        ((Const *)res->getSubExp2())->setInt(((Const *)res->getSubExp2())->getInt() * m);
        bMod = true;
        return res;
    }

    // check for !(a == b) becomes a != b
    if (op == opLNot && opSub1 == opEquals) {
        res = ((Unary *)res)->getSubExp1();
        res->setOper(opNotEqual);
        bMod = true;
        return res;
    }

    // check for !(a != b) becomes a == b
    if (op == opLNot && opSub1 == opNotEqual) {
        res = ((Unary *)res)->getSubExp1();
        res->setOper(opEquals);
        bMod = true;
        return res;
    }

#if 0 // FIXME! ADHOC TA assumed!
    // check for (exp + x) + n where exp is a pointer to a compound type becomes (exp + n) + x
    if (    op == opPlus &&
            subExp1->getOper() == opPlus &&
            subExp1->getSubExp1()->getType() &&
            subExp2->getOper() == opIntConst) {
        SharedType ty = subExp1->getSubExp1()->getType();
        if (ty->resolvesToPointer() &&
                ty->asPointer()->getPointsTo()->resolvesToCompound()) {
            res = Binary::get(opPlus, subExp1->getSubExp1(), subExp2);
            res = Binary::get(opPlus, res, subExp1->getSubExp2());
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
        Instruction *def = ((RefExp *)subExp1)->getDef();
        if (def)
            ty = def->getTypeFor(((RefExp *)subExp1)->getSubExp1());
    }
    if (op == opPlus && ty && ty->resolvesToPointer() && ty->asPointer()->getPointsTo()->resolvesToCompound() &&
        opSub2 == opIntConst) {
        unsigned n = (unsigned)((Const *)subExp2)->getInt();
        std::shared_ptr<CompoundType> c = ty->asPointer()->getPointsTo()->asCompound();
        res = convertFromOffsetToCompound(subExp1,c,n);
        if(res) {
                LOG_VERBOSE(1) << "(trans1) replacing " << this << " with " << res << "\n";
                bMod = true;
                return res;
        }
    }

#if 0 // FIXME: ADHOC TA assumed
    // check for exp + x where exp is a pointer to an array
    // becomes &exp[x / b] + (x % b) where b is the size of the base type in bytes
    if (    op == opPlus &&
            subExp1->getType()) {
        Exp *x = subExp2;
        Exp *l = subExp1;
        SharedType ty = l->getType();
        if (    ty && ty->resolvesToPointer() &&
                ty->asPointer()->getPointsTo()->resolvesToArray()) {
            auto a = ty->asPointer()->getPointsTo()->asArray();
            int b = a->getBaseType()->getSize() / 8;
            int br = a->getBaseType()->getSize() % 8;
            assert(br == 0);
            if (x->getOper() != opIntConst || ((Const*)x)->getInt() >= b || a->getBaseType()->isArray()) {
                res = Binary::get(opPlus,
                                  new Unary(opAddrOf,
                                            Binary::get(opArrayIndex,
                                                        Location::memOf(l->clone()),
                                                        Binary::get(opDiv, x->clone(), new Const(b)))),
                                  Binary::get(opMod, x->clone(), new Const(b)));
                if (VERBOSE)
                    LOG << "replacing " << this << " with " << res << "\n";
                if (l->getOper() == opSubscript) {
                    RefExp *r = (RefExp*)l;
                    if (r->getDef() && r->getDef()->isPhi()) {
                        PhiAssign *pa = (PhiAssign*)r->getDef();
                        LOG << "argh: " << pa->getStmtAt(1) << "\n";
                    }
                }
                bMod = true;
                return res;
            }
        }
    }
#endif

    if (op == opFMinus && subExp1->getOper() == opFltConst && ((Const *)subExp1)->getFlt() == 0.0) {
        res = new Unary(opFNeg, subExp2);
        bMod = true;
        return res;
    }

    if ((op == opPlus || op == opMinus) && (subExp1->getOper() == opMults || subExp1->getOper() == opMult) &&
        subExp2->getOper() == opIntConst && subExp1->getSubExp2()->getOper() == opIntConst) {
        int n1 = ((Const *)subExp2)->getInt();
        int n2 = ((Const *)subExp1->getSubExp2())->getInt();
        if (n1 == n2) {
            res = Binary::get(subExp1->getOper(), Binary::get(op, subExp1->getSubExp1()->clone(), new Const(1)),
                              new Const(n1));
            bMod = true;
            return res;
        }
    }

    if ((op == opPlus || op == opMinus) && subExp1->getOper() == opPlus && subExp2->getOper() == opIntConst &&
        (subExp1->getSubExp2()->getOper() == opMults || subExp1->getSubExp2()->getOper() == opMult) &&
        subExp1->getSubExp2()->getSubExp2()->getOper() == opIntConst) {
        int n1 = ((Const *)subExp2)->getInt();
        int n2 = ((Const *)subExp1->getSubExp2()->getSubExp2())->getInt();
        if (n1 == n2) {
            res = Binary::get(opPlus, subExp1->getSubExp1(),
                              Binary::get(subExp1->getSubExp2()->getOper(),
                                          Binary::get(op, subExp1->getSubExp2()->getSubExp1()->clone(), new Const(1)),
                                          new Const(n1)));
            bMod = true;
            return res;
        }
    }

    // check for ((x * a) + (y * b)) / c where a, b and c are all integers and a and b divide evenly by c
    // becomes: (x * a/c) + (y * b/c)
    if (op == opDiv && subExp1->getOper() == opPlus && subExp2->getOper() == opIntConst &&
        subExp1->getSubExp1()->getOper() == opMult && subExp1->getSubExp2()->getOper() == opMult &&
        subExp1->getSubExp1()->getSubExp2()->getOper() == opIntConst &&
        subExp1->getSubExp2()->getSubExp2()->getOper() == opIntConst) {
        int a = ((Const *)subExp1->getSubExp1()->getSubExp2())->getInt();
        int b = ((Const *)subExp1->getSubExp2()->getSubExp2())->getInt();
        int c = ((Const *)subExp2)->getInt();
        if ((a % c) == 0 && (b % c) == 0) {
            res = Binary::get(opPlus, Binary::get(opMult, subExp1->getSubExp1()->getSubExp1(), new Const(a / c)),
                              Binary::get(opMult, subExp1->getSubExp2()->getSubExp1(), new Const(b / c)));
            bMod = true;
            return res;
        }
    }

    // check for ((x * a) + (y * b)) % c where a, b and c are all integers
    // becomes: (y * b) % c if a divides evenly by c
    // becomes: (x * a) % c if b divides evenly by c
    // becomes: 0            if both a and b divide evenly by c
    if (op == opMod && subExp1->getOper() == opPlus && subExp2->getOper() == opIntConst &&
        subExp1->getSubExp1()->getOper() == opMult && subExp1->getSubExp2()->getOper() == opMult &&
        subExp1->getSubExp1()->getSubExp2()->getOper() == opIntConst &&
        subExp1->getSubExp2()->getSubExp2()->getOper() == opIntConst) {
        int a = ((Const *)subExp1->getSubExp1()->getSubExp2())->getInt();
        int b = ((Const *)subExp1->getSubExp2()->getSubExp2())->getInt();
        int c = ((Const *)subExp2)->getInt();
        if ((a % c) == 0 && (b % c) == 0) {
            res = new Const(0);
            bMod = true;
            return res;
        }
        if ((a % c) == 0) {
            res = Binary::get(opMod, subExp1->getSubExp2()->clone(), new Const(c));
            bMod = true;
            return res;
        }
        if ((b % c) == 0) {
            res = Binary::get(opMod, subExp1->getSubExp1()->clone(), new Const(c));
            bMod = true;
            return res;
        }
    }

    // Check for 0 - (0 <u exp1) & exp2 => exp2
    if (op == opBitAnd && opSub1 == opMinus) {
        Exp *leftOfMinus = ((Binary *)subExp1)->getSubExp1();
        if (leftOfMinus->isIntConst() && ((Const *)leftOfMinus)->getInt() == 0) {
            Exp *rightOfMinus = ((Binary *)subExp1)->getSubExp2();
            if (rightOfMinus->getOper() == opLessUns) {
                Exp *leftOfLess = ((Binary *)rightOfMinus)->getSubExp1();
                if (leftOfLess->isIntConst() && ((Const *)leftOfLess)->getInt() == 0) {
                    res = getSubExp2();
                    bMod = true;
                    return res;
                }
            }
        }
    }

    // Replace opSize(n, loc) with loc and set the type if needed
    if (op == opSize && subExp2->isLocation()) {
#if 0 // FIXME: ADHOC TA assumed here
        Location *loc = (Location*)subExp2;
        unsigned n = (unsigned)((Const*)subExp1)->getInt();
        SharedType ty = loc->getType();
        if (ty == nullptr)
            loc->setType(SizeType::get(n));
        else if (ty->getSize() != n)
            ty->setSize(n);
#endif
        res = ((Binary *)res)->getSubExp2();
        bMod = true;
        return res;
    }

    return res;
}

Exp *Ternary::polySimplify(bool &bMod) {
    Exp *res = this;

    subExp1 = subExp1->polySimplify(bMod);
    subExp2 = subExp2->polySimplify(bMod);
    subExp3 = subExp3->polySimplify(bMod);

    // p ? 1 : 0 -> p
    if (op == opTern && subExp2->getOper() == opIntConst && subExp3->getOper() == opIntConst) {
        Const *s2 = (Const *)subExp2;
        Const *s3 = (Const *)subExp3;

        if (s2->getInt() == 1 && s3->getInt() == 0) {
            res = this->getSubExp1();
            bMod = true;
            return res;
        }
    }

    // 1 ? x : y -> x
    if (op == opTern && subExp1->getOper() == opIntConst && ((Const *)subExp1)->getInt() == 1) {
        res = this->getSubExp2();
        bMod = true;
        return res;
    }

    // 0 ? x : y -> y
    if (op == opTern && subExp1->getOper() == opIntConst && ((Const *)subExp1)->getInt() == 0) {
        res = this->getSubExp3();
        bMod = true;
        return res;
    }

    if ((op == opSgnEx || op == opZfill) && subExp3->getOper() == opIntConst) {
        res = this->getSubExp3();
        bMod = true;
        return res;
    }

    if (op == opFsize && subExp3->getOper() == opItof && *subExp1 == *subExp3->getSubExp2() &&
        *subExp2 == *subExp3->getSubExp1()) {
        res = this->getSubExp3();
        bMod = true;
        return res;
    }

    if (op == opFsize && subExp3->getOper() == opFltConst) {
        res = this->getSubExp3();
        bMod = true;
        return res;
    }

    if (op == opItof && subExp3->getOper() == opIntConst && subExp2->getOper() == opIntConst &&
        ((Const *)subExp2)->getInt() == 32) {
        unsigned n = ((Const *)subExp3)->getInt();
        res = new Const(*(float *)&n);
        bMod = true;
        return res;
    }

    if (op == opFsize && subExp3->getOper() == opMemOf && subExp3->getSubExp1()->getOper() == opIntConst) {
        ADDRESS u = ((Const *)subExp3->getSubExp1())->getAddr();
        Location *l = dynamic_cast<Location *>(subExp3);
        UserProc *p = l->getProc();
        if (p) {
            Prog *prog = p->getProg();
            bool ok;
            double d = prog->getFloatConstant(u, ok, ((Const *)subExp1)->getInt());
            if (ok) {
                if (VERBOSE)
                    LOG << "replacing " << subExp3 << " with " << d << " in " << this << "\n";
                subExp3 = new Const(d);
                bMod = true;
                return res;
            }
        }
    }

    if (op == opTruncu && subExp3->isIntConst()) {
        int from = ((Const *)subExp1)->getInt();
        int to = ((Const *)subExp2)->getInt();
        unsigned int val = ((Const *)subExp3)->getInt();
        if (from == 32) {
            if (to == 16) {
                res = new Const(ADDRESS::g(val & 0xffff));
                bMod = true;
                return res;
            }
            if (to == 8) {
                res = new Const(ADDRESS::g(val & 0xff));
                bMod = true;
                return res;
            }
        }
    }

    if (op == opTruncs && subExp3->isIntConst()) {
        int from = ((Const *)subExp1)->getInt();
        int to = ((Const *)subExp2)->getInt();
        int val = ((Const *)subExp3)->getInt();
        if (from == 32) {
            if (to == 16) {
                res = new Const(val & 0xffff);
                bMod = true;
                return res;
            }
            if (to == 8) {
                res = new Const(val & 0xff);
                bMod = true;
                return res;
            }
        }
    }

    return res;
}

Exp *TypedExp::polySimplify(bool &bMod) {
    Exp *res = this;

    if (subExp1->getOper() == opRegOf) {
        // type cast on a reg of.. hmm.. let's remove this
        res = ((Unary *)res)->getSubExp1();
        bMod = true;
        return res;
    }

    subExp1 = subExp1->simplify();
    return res;
}

Exp *RefExp::polySimplify(bool &bMod) {
    Exp *res = this;

    Exp *tmp = subExp1->polySimplify(bMod);
    if (bMod) {
        subExp1 = tmp;
        return res;
    }

    /* This is a nasty hack.  We assume that %DF{0} is 0.  This happens when string instructions are used without first
         * clearing the direction flag.  By convention, the direction flag is assumed to be clear on entry to a
     * procedure.
         */
    if (subExp1->getOper() == opDF && def == nullptr) {
        res = Const::get(int(0));
        bMod = true;
        return res;
    }

    // another hack, this time for aliasing
    // FIXME: do we really want this now? Pentium specific, and only handles ax/eax (not al or ah)
    if (subExp1->isRegN(0) &&                                               // r0 (ax)
        def && def->isAssign() && ((Assign *)def)->getLeft()->isRegN(24)) { // r24 (eax)
        res = new TypedExp(IntegerType::get(16), new RefExp(Location::regOf(24), def));
        bMod = true;
        return res;
    }

    // Was code here for bypassing phi statements that are now redundant

    return res;
}

/***************************************************************************/ /**
  *
  * \brief        Just do addressof simplification: a[ m[ any ]] == any, m[ a[ any ]] = any, and also
  *               a[ size m[ any ]] == any
  * \todo            Replace with a visitor some day
  * \returns            Ptr to the simplified expression
  ******************************************************************************/
Exp *Unary::simplifyAddr() {
    Exp *sub;
    if (op == opMemOf && subExp1->isAddrOf()) {
        Unary *s = (Unary *)getSubExp1();
        return s->getSubExp1();
    }
    if (op != opAddrOf) {
        // Not a[ anything ]. Recurse
        subExp1 = subExp1->simplifyAddr();
        return this;
    }
    if (subExp1->getOper() == opMemOf) {
        Unary *s = (Unary *)getSubExp1();
        return s->getSubExp1();
    }
    if (subExp1->getOper() == opSize) {
        sub = subExp1->getSubExp2();
        if (sub->getOper() == opMemOf) {
            // Remove the a[
            Binary *b = (Binary *)getSubExp1();
            // Remove the size[
            Unary *u = (Unary *)b->getSubExp2();
            // Remove the m[
            return u->getSubExp1();
        }
    }

    // a[ something else ]. Still recurse, just in case
    subExp1 = subExp1->simplifyAddr();
    return this;
}

Exp *Binary::simplifyAddr() {
    assert(subExp1 && subExp2);

    subExp1 = subExp1->simplifyAddr();
    subExp2 = subExp2->simplifyAddr();
    return this;
}

Exp *Ternary::simplifyAddr() {
    subExp1 = subExp1->simplifyAddr();
    subExp2 = subExp2->simplifyAddr();
    subExp3 = subExp3->simplifyAddr();
    return this;
}

const char *Exp::getOperName() const { return operStrings[op]; }

QString Exp::toString() const {
    QString res;
    QTextStream os(&res);
    this->print(os);
    return res;
}

/***************************************************************************/ /**
  *
  * \brief  Print an infix representation of the object to the given file stream, with its type in \<angle
  *         brackets\>.
  * \param os Output stream to send the output to
  ******************************************************************************/
void Exp::printt(QTextStream &os /*= cout*/) const {
    print(os);
    if (op != opTypedExp)
        return;
    SharedType t = ((TypedExp *)this)->getType();
    os << "<" << t->getSize();
    /*      switch (t->getType()) {
                case INTEGER:
                        if (t->getSigned())
                                                os << "i";                // Integer
                        else
                                                os << "u"; break;        // Unsigned
                case FLOATP:    os << "f"; break;
                case DATA_ADDRESS: os << "pd"; break;    // Pointer to Data
                case FUNC_ADDRESS: os << "pc"; break;    // Pointer to Code
                case VARARGS:    os << "v"; break;
                case TBOOLEAN:     os << "b"; break;
                case UNKNOWN:    os << "?"; break;
                case TVOID:        break;
        } */
    os << ">";
}

/***************************************************************************/ /**
  *
  * \brief        Print an infix representation of the object to the given file stream, but convert r[10] to r10 and
  *                      v[5] to v5
  * \note Never modify this function to emit debugging info; the back ends rely on this being clean to emit
  *                      correct C.  If debugging is desired, use operator<<
  * \param os Output stream to send the output to
  ******************************************************************************/
void Exp::printAsHL(QTextStream &os /*= cout*/) {
    QString tgt;
    QTextStream ost(&tgt);
    ost << this; // Print to the string stream
    if ((tgt.length() >= 4) && (tgt[1] == '[')) {
        // r[nn]; change to rnn
        tgt.remove(1, 1);           // '['
        tgt.remove(tgt.length() - 1); // ']'
    }
    os << tgt; // Print to the output stream
}

/***************************************************************************/ /**
  *
  * \brief Output operator for Exp*
  * \param os output stream to send to
  * \param p ptr to Exp to print to the stream
  * \returns copy of os (for concatenation)
  ******************************************************************************/
QTextStream &operator<<(QTextStream &os, const Exp *p) {
#if 1
    // Useful for debugging, but can clutter the output
    p->printt(os);
#else
    p->print(os);
#endif
    return os;
}

/***************************************************************************/ /**
  *
  * \brief       Replace succ(r[k]) by r[k+1]
  * \note        Could change top level expression
  * \returns     Fixed expression
  ******************************************************************************/
Exp *Exp::fixSuccessor() {
    bool change;
    Exp *result;
    UniqExp search_expression(Unary::get(opSuccessor, Location::regOf(Terminal::get(opWild))));
    // Assume only one successor function in any 1 expression
    if (search(*search_expression, result)) {
        // Result has the matching expression, i.e. succ(r[K])
        Exp *sub1 = ((Unary *)result)->getSubExp1();
        assert(sub1->getOper() == opRegOf);
        Exp *sub2 = ((Unary *)sub1)->getSubExp1();
        assert(sub2->getOper() == opIntConst);
        // result     sub1    sub2
        // succ(      r[   Const K    ])
        // Note: we need to clone the r[K] part, since it will be ;//deleted as
        // part of the searchReplace below
        Unary *replace = (Unary *)sub1->clone();
        Const *c = (Const *)replace->getSubExp1();
        c->setInt(c->getInt() + 1); // Do the increment
        Exp *res = searchReplace(*result, replace, change);
        return res;
    }
    return this;
}

/***************************************************************************/ /**
  *
  * \brief        Remove size operations such as zero fill, sign extend
  * \note            Could change top level expression
  * \note            Does not handle truncation at present
  * \returns            Fixed expression
  ******************************************************************************/
Exp *Exp::killFill() {
    static Ternary srch1(opZfill, new Terminal(opWild), new Terminal(opWild), new Terminal(opWild));
    static Ternary srch2(opSgnEx, new Terminal(opWild), new Terminal(opWild), new Terminal(opWild));
    Exp *res = this;
    std::list<Exp **> result;
    doSearch(srch1, res, result, false);
    doSearch(srch2, res, result, false);
    std::list<Exp **>::iterator it;
    for (it = result.begin(); it != result.end(); it++) {
        // Kill the sign extend bits
        **it = ((Ternary *)(**it))->getSubExp3();
    }
    return res;
}

bool Exp::isTemp() {
    if (op == opTemp)
        return true;
    if (op != opRegOf)
        return false;
    // Some old code has r[tmpb] instead of just tmpb
    Exp *sub = ((Unary *)this)->getSubExp1();
    return sub->op == opTemp;
}

// allZero is set if all subscripts in the whole expression are null or implicit; otherwise cleared
Exp *Exp::removeSubscripts(bool &allZero) {
    Exp *e = this;
    LocationSet locs;
    e->addUsedLocs(locs);
    LocationSet::iterator xx;
    allZero = true;
    for (xx = locs.begin(); xx != locs.end(); xx++) {
        if ((*xx)->getOper() == opSubscript) {
            RefExp *r1 = (RefExp *)*xx;
            Instruction *def = r1->getDef();
            if (!(def == nullptr || def->getNumber() == 0)) {
                allZero = false;
            }
            bool change;
            e = e->searchReplaceAll(**xx, r1->getSubExp1() /*->clone()*/,
                                    change); // TODO: what happens when clone is restored here ?
        }
    }
    return e;
}

// FIXME: if the wrapped expression does not convert to a location, the result is subscripted, which is probably not
// what is wanted!
Exp *Exp::fromSSAleft(UserProc *proc, Instruction *d) {
    RefExp *r = new RefExp(this, d); // "Wrap" in a ref
    return r->accept(new ExpSsaXformer(proc));
}

// A helper class for comparing Exp*'s sensibly
bool lessExpStar::operator()(const Exp *x, const Exp *y) const {
    return (*x < *y); // Compare the actual Exps
}
bool lessExpShared::operator()(const std::shared_ptr<Exp> &x, const std::shared_ptr<Exp> &y) const {
    return (*x < *y); // Compare the actual Exps
}

bool lessTI::operator()(const Exp *x, const Exp *y) const {
    return (*x << *y); // Compare the actual Exps
}

//    //    //    //    //    //
//    genConstraints    //
//    //    //    //    //    //

Exp *Exp::genConstraints(Exp * /*result*/) {
    // Default case, no constraints -> return true
    return new Terminal(opTrue);
}

Exp *Const::genConstraints(Exp *result) {
    if (result->isTypeVal()) {
        // result is a constant type, or possibly a partial type such as ptr(alpha)
        SharedType t = ((TypeVal *)result)->getType();
        bool match = false;
        switch (op) {
        case opLongConst:
        // An integer constant is compatible with any size of integer, as long is it is in the right range
        // (no test yet) FIXME: is there an endianness issue here?
        case opIntConst:
            match = t->isInteger();
            // An integer constant can also match a pointer to something.  Assume values less than 0x100 can't be a
            // pointer
            if ((unsigned)u.i >= 0x100)
                match |= t->isPointer();
            // We can co-erce 32 bit constants to floats
            match |= t->isFloat();
            break;
        case opStrConst:
            if(t->isPointer()) {
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
            return Binary::get(opEquals, new Unary(opTypeOf, this), result->clone());
        } else
            // Doesn't match
            return new Terminal(opFalse);
    }
    // result is a type variable, which is constrained by this constant
    SharedType t;
    switch (op) {
    case opIntConst: {
        // We have something like local1 = 1234.  Either they are both integer, or both pointer
        SharedType intt = IntegerType::get(0);
        SharedType alph = PointerType::newPtrAlpha();
        return Binary::get(
            opOr, Binary::get(
                      opAnd, Binary::get(opEquals, result->clone(), new TypeVal(intt)),
                      Binary::get(opEquals,
                                  new Unary(opTypeOf,
                                            // Note: don't clone 'this', so we can change the Const after type analysis!
                                            this),
                                  new TypeVal(intt))),
            Binary::get(opAnd, Binary::get(opEquals, result->clone(), new TypeVal(alph)),
                        Binary::get(opEquals, new Unary(opTypeOf, this), new TypeVal(alph))));
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
    TypeVal *tv = new TypeVal(t);
    Exp *e = Binary::get(opEquals, result->clone(), tv);
    return e;
}

Exp *Unary::genConstraints(Exp *result) {
    if (result->isTypeVal()) {
        // TODO: need to check for conflicts
        return new Terminal(opTrue);
    }

    switch (op) {
    case opRegOf:
    case opParam: // Should be no params at constraint time
    case opGlobal:
    case opLocal:
        return Binary::get(opEquals, new Unary(opTypeOf, this->clone()), result->clone());
    default:
        break;
    }
    return new Terminal(opTrue);
}

Exp *Ternary::genConstraints(Exp *result) {
    SharedType argHasToBe = nullptr;
    SharedType retHasToBe = nullptr;
    switch (op) {
    case opFsize:
    case opItof:
    case opFtoi:
    case opSgnEx: {
        assert(subExp1->isIntConst());
        assert(subExp2->isIntConst());
        int fromSize = ((Const *)subExp1)->getInt();
        int toSize = ((Const *)subExp2)->getInt();
        // Fall through
        switch (op) {
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
    Exp *res = nullptr;
    if (retHasToBe) {
        if (result->isTypeVal()) {
            // result is a constant type, or possibly a partial type such as
            // ptr(alpha)
            SharedType t = ((TypeVal *)result)->getType();
            // Compare broad types
            if (!(*retHasToBe *= *t))
                return new Terminal(opFalse);
            // else just constrain the arg
        } else {
            // result is a type variable, constrained by this Ternary
            res = Binary::get(opEquals, result, new TypeVal(retHasToBe));
        }
    }
    if (argHasToBe) {
        // Constrain the argument
        Exp *con = subExp3->genConstraints(new TypeVal(argHasToBe));
        if (res)
            res = Binary::get(opAnd, res, con);
        else
            res = con;
    }
    if (res == nullptr)
        return new Terminal(opTrue);
    return res;
}

Exp *RefExp::genConstraints(Exp *result) {
    OPER subOp = subExp1->getOper();
    switch (subOp) {
    case opRegOf:
    case opParam:
    case opGlobal:
    case opLocal:
        return Binary::get(opEquals, new Unary(opTypeOf, this->clone()), result->clone());
    default:
        break;
    }
    return new Terminal(opTrue);
}

// Return a constraint that my subexpressions have to be of type typeval1 and typeval2 respectively
Exp *Binary::constrainSub(TypeVal *typeVal1, TypeVal *typeVal2) {
    assert(subExp1 && subExp2);

    Exp *con1 = subExp1->genConstraints(typeVal1);
    Exp *con2 = subExp2->genConstraints(typeVal2);
    return Binary::get(opAnd, con1, con2);
}

Exp *Binary::genConstraints(Exp *result) {
    assert(subExp1 && subExp2);

    SharedType restrictTo = nullptr;
    if (result->isTypeVal())
        restrictTo = ((TypeVal *)result)->getType();
    Exp *res = nullptr;
    auto intType = IntegerType::get(0); // Wild size (=0)
    TypeVal intVal(intType);
    switch (op) {
    case opFPlus:
    case opFMinus:
    case opFMult:
    case opFDiv: {
        if (restrictTo && !restrictTo->isFloat())
            // Result can only be float
            return new Terminal(opFalse);

        // MVE: what about sizes?
        auto ft = FloatType::get();
        TypeVal *ftv = new TypeVal(ft);
        res = constrainSub(ftv, ftv);
        if (!restrictTo)
            // Also constrain the result
            res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), ftv));
        return res;
    }

    case opBitAnd:
    case opBitOr:
    case opBitXor: {
        if (restrictTo && !restrictTo->isInteger())
            // Result can only be integer
            return new Terminal(opFalse);

        // MVE: What about sizes?
        auto it = IntegerType::get(STD_SIZE, 0);
        TypeVal *itv = new TypeVal(it);
        res = constrainSub(itv, itv);
        if (!restrictTo)
            // Also constrain the result
            res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), itv));
        return res;
    }

    case opPlus: {
        // A pointer to anything
        SharedType ptrType = PointerType::newPtrAlpha();
        TypeVal ptrVal(ptrType); // Type value of ptr to anything
        if (!restrictTo || restrictTo->isInteger()) {
            // int + int -> int
            res = constrainSub(&intVal, &intVal);
            if (!restrictTo)
                res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), intVal.clone()));
        }

        if (!restrictTo || restrictTo->isPointer()) {
            // ptr + int -> ptr
            Exp *res2 = constrainSub(&ptrVal, &intVal);
            if (!restrictTo)
                res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal.clone()));
            if (res)
                res = Binary::get(opOr, res, res2);
            else
                res = res2;

            // int + ptr -> ptr
            res2 = constrainSub(&intVal, &ptrVal);
            if (!restrictTo)
                res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal.clone()));
            if (res)
                res = Binary::get(opOr, res, res2);
            else
                res = res2;
        }

        if (res)
            return res->simplify();
        return new Terminal(opFalse);
    }

    case opMinus: {
        SharedType ptrType = PointerType::newPtrAlpha();
        TypeVal ptrVal(ptrType);
        if (!restrictTo || restrictTo->isInteger()) {
            // int - int -> int
            res = constrainSub(&intVal, &intVal);
            if (!restrictTo)
                res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), intVal.clone()));

            // ptr - ptr -> int
            Exp *res2 = constrainSub(&ptrVal, &ptrVal);
            if (!restrictTo)
                res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), intVal.clone()));
            if (res)
                res = Binary::get(opOr, res, res2);
            else
                res = res2;
        }

        if (!restrictTo || restrictTo->isPointer()) {
            // ptr - int -> ptr
            Exp *res2 = constrainSub(&ptrVal, &intVal);
            if (!restrictTo)
                res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal.clone()));
            if (res)
                res = Binary::get(opOr, res, res2);
            else
                res = res2;
        }

        if (res)
            return res->simplify();
        return new Terminal(opFalse);
    }

    case opSize: {
        // This used to be considered obsolete, but now, it is used to carry the size of memOf's from the decoder to
        // here
        assert(subExp1->isIntConst());
        int sz = ((Const *)subExp1)->getInt();
        if (restrictTo) {
            int rsz = restrictTo->getSize();
            if (rsz == 0) {
                // This is now restricted to the current restrictTo, but
                // with a known size
                SharedType it = restrictTo->clone();
                it->setSize(sz);
                return Binary::get(opEquals, new Unary(opTypeOf, subExp2), new TypeVal(it));
            }
            return new Terminal((rsz == sz) ? opTrue : opFalse);
        }
        // We constrain the size but not the basic type
        return Binary::get(opEquals, result->clone(), new TypeVal(SizeType::get(sz)));
    }

    default:
        break;
    }
    return new Terminal(opTrue);
}

Exp *Location::polySimplify(bool &bMod) {
    Exp *res = Unary::polySimplify(bMod);

    if (res->getOper() == opMemOf && res->getSubExp1()->getOper() == opAddrOf) {
        if (VERBOSE)
            LOG << "polySimplify " << res << "\n";
        res = res->getSubExp1()->getSubExp1();
        bMod = true;
        return res;
    }

    // check for m[a[loc.x]] becomes loc.x
    if (res->getOper() == opMemOf && res->getSubExp1()->getOper() == opAddrOf &&
        res->getSubExp1()->getSubExp1()->getOper() == opMemberAccess) {
        res = subExp1->getSubExp1();
        bMod = true;
        return res;
    }

    return res;
}

void Location::getDefinitions(LocationSet &defs) {
    // This is a hack to fix aliasing (replace with something general)
    // FIXME! This is x86 specific too. Use -O for overlapped registers!
    if (op == opRegOf && ((Const *)subExp1)->getInt() == 24) {
        defs.insert(Location::regOf(0));
    }
}

QString Const::getFuncName() const { return u.pp->getName(); }

Exp *Unary::simplifyConstraint() {
    subExp1 = subExp1->simplifyConstraint();
    return this;
}

Exp *Binary::simplifyConstraint() {
    assert(subExp1 && subExp2);

    subExp1 = subExp1->simplifyConstraint();
    subExp2 = subExp2->simplifyConstraint();
    switch (op) {
    case opEquals: {
        if (subExp1->isTypeVal() && subExp2->isTypeVal()) {
            // FIXME: ADHOC TA assumed
            SharedType t1 = ((TypeVal *)subExp1)->getType();
            SharedType t2 = ((TypeVal *)subExp2)->getType();
            if (!t1->isPointerToAlpha() && !t2->isPointerToAlpha()) {
                delete this;
                if (*t1 == *t2)
                    return new Terminal(opTrue);
                else
                    return new Terminal(opFalse);
            }
        }
        break;
    }

    case opOr:
    case opAnd:
    case opNot:
        return simplify();
    default:
        break;
    }
    return this;
}

//    //    //    //    //    //    //    //
//                            //
//       V i s i t i n g        //
//                            //
//    //    //    //    //    //    //    //
bool Unary::accept(ExpVisitor *v) {
    bool override, ret = v->visit(this, override);
    if (override)
        return ret; // Override the rest of the accept logic
    if (ret)
        ret = subExp1->accept(v);
    return ret;
}

bool Binary::accept(ExpVisitor *v) {
    assert(subExp1 && subExp2);

    bool override, ret = v->visit(this, override);
    if (override)
        return ret;
    if (ret)
        ret = subExp1->accept(v);
    if (ret)
        ret = subExp2->accept(v);
    return ret;
}

bool Ternary::accept(ExpVisitor *v) {
    bool override, ret = v->visit(this, override);
    if (override)
        return ret;
    if (ret)
        ret = subExp1->accept(v);
    if (ret)
        ret = subExp2->accept(v);
    if (ret)
        ret = subExp3->accept(v);
    return ret;
}

// All the Unary derived accept functions look the same, but they have to be repeated because the particular visitor
// function called each time is different for each class (because "this" is different each time)
bool TypedExp::accept(ExpVisitor *v) {
    bool override, ret = v->visit(this, override);
    if (override)
        return ret;
    if (ret)
        ret = subExp1->accept(v);
    return ret;
}
bool FlagDef::accept(ExpVisitor *v) {
    bool override, ret = v->visit(this, override);
    if (override)
        return ret;
    if (ret)
        ret = subExp1->accept(v);
    return ret;
}
bool RefExp::accept(ExpVisitor *v) {
    bool override;
    bool ret = v->visit(*this, override);
    if (override)
        return ret;
    if (ret)
        ret = subExp1->accept(v);
    return ret;
}
bool Location::accept(ExpVisitor *v) {
    bool override = false, ret = v->visit(this, override);
    if (override)
        return ret;
    if (ret)
        ret &= subExp1->accept(v);
    return ret;
}

// The following are similar, but don't have children that have to accept visitors
bool Terminal::accept(ExpVisitor *v) { return v->visit(this); }
bool Const::accept(ExpVisitor *v) { return v->visit(this); }
bool TypeVal::accept(ExpVisitor *v) { return v->visit(this); }

// FixProcVisitor class

void Exp::fixLocationProc(UserProc *p) {
    // All locations are supposed to have a pointer to the enclosing UserProc that they are a location of. Sometimes,
    // you have an arbitrary expression that may not have all its procs set. This function fixes the procs for all
    // Location subexpresssions.
    FixProcVisitor fpv;
    fpv.setProc(p);
    accept(&fpv);
}

// GetProcVisitor class

UserProc *Exp::findProc() {
    GetProcVisitor gpv;
    accept(&gpv);
    return gpv.getProc();
}

void Exp::setConscripts(int n, bool bClear) {
    SetConscripts sc(n, bClear);
    accept(&sc);
}

// Strip size casts from an Exp
Exp *Exp::stripSizes() {
    SizeStripper ss;
    return accept(&ss);
}

Exp *Unary::accept(ExpModifier *v) {
    // This Unary will be changed in *either* the pre or the post visit. If it's changed in the preVisit step, then
    // postVisit doesn't care about the type of ret. So let's call it a Unary, and the type system is happy
    bool recur;
    Unary *ret = (Unary *)v->preVisit(this, recur);
    if (recur)
        subExp1 = subExp1->accept(v);
    return v->postVisit(ret);
}
Exp *Binary::accept(ExpModifier *v) {
    assert(subExp1 && subExp2);

    bool recur;
    Exp *ret = v->preVisit(this, recur);
    if (recur)
        subExp1 = subExp1->accept(v);
    if (recur)
        subExp2 = subExp2->accept(v);
    Binary *bret = dynamic_cast<Binary *>(ret);
    Unary *uret = dynamic_cast<Unary *>(ret);
    if(bret)
        return v->postVisit(bret);
    if(uret)
        return v->postVisit(uret);
    Q_ASSERT(false);
    return nullptr;
}
Exp *Ternary::accept(ExpModifier *v) {
    bool recur;
    Ternary *ret = (Ternary *)v->preVisit(this, recur);
    if (recur)
        subExp1 = subExp1->accept(v);
    if (recur)
        subExp2 = subExp2->accept(v);
    if (recur)
        subExp3 = subExp3->accept(v);
    return v->postVisit(ret);
}

Exp *Location::accept(ExpModifier *v) {
    // This looks to be the same source code as Unary::accept, but the type of "this" is different, which is all
    // important here!  (it makes a call to a different visitor member function).
    bool recur;
    Exp *ret = v->preVisit(this, recur);
    if (recur)
        subExp1 = subExp1->accept(v);
    Location * loc_ret = dynamic_cast<Location *>(ret);
    if(loc_ret)
        return v->postVisit(loc_ret);
    RefExp * ref_ret = dynamic_cast<RefExp *>(ret);
    if(ref_ret)
        return v->postVisit(ref_ret);
    assert(false);
    return nullptr;
}

Exp *RefExp::accept(ExpModifier *v) {
    bool recur;
    RefExp *ret = (RefExp *)v->preVisit(this, recur);
    if (recur)
        subExp1 = subExp1->accept(v);
    return v->postVisit(ret);
}

Exp *FlagDef::accept(ExpModifier *v) {
    bool recur;
    FlagDef *ret = (FlagDef *)v->preVisit(this, recur);
    if (recur)
        subExp1 = subExp1->accept(v);
    return v->postVisit(ret);
}

Exp *TypedExp::accept(ExpModifier *v) {
    bool recur;
    TypedExp *ret = (TypedExp *)v->preVisit(this, recur);
    if (recur)
        subExp1 = subExp1->accept(v);
    return v->postVisit(ret);
}

Exp *Terminal::accept(ExpModifier *v) {
    // This is important if we need to modify terminals
    Exp *val = v->preVisit(this);
    Terminal *term_res = dynamic_cast<Terminal *>(val);
    if(term_res)
        return v->postVisit(term_res);

    RefExp *ref_res = dynamic_cast<RefExp *>(val);
    if(ref_res)
        return v->postVisit(ref_res);
    assert(false);
    return nullptr;
}

Exp *Const::accept(ExpModifier *v) { return v->postVisit((Const *)v->preVisit(this)); }

Exp *TypeVal::accept(ExpModifier *v) { return v->postVisit((TypeVal *)v->preVisit(this)); }
QTextStream &alignStream(QTextStream &str,int align) {
    str << qSetFieldWidth(align) << " " << qSetFieldWidth(0);
    return str;
}
void child(Exp *e, int ind) {
    if (e == nullptr) {
        alignStream(LOG_STREAM(),ind+4) << "<nullptr>\n";
        LOG_STREAM().flush();
        return;
    }
    void *vt = *(void **)e;
    if (vt == nullptr) {
        alignStream(LOG_STREAM(),ind+4) << "<nullptr VT>\n";
        LOG_STREAM().flush();
        return;
    }
    e->printx(ind + 4);
}

void Unary::printx(int ind) const {
    alignStream(LOG_STREAM(),ind) << operStrings[op] << "\n";
    LOG_STREAM().flush();
    child(subExp1, ind);
}

void Binary::printx(int ind) const {
    assert(subExp1 && subExp2);
    alignStream(LOG_STREAM(),ind) << operStrings[op] << "\n";
    LOG_STREAM().flush();
    child(subExp1, ind);
    child(subExp2, ind);
}

void Ternary::printx(int ind) const {
    alignStream(LOG_STREAM(),ind) << operStrings[op] << "\n";
    child(subExp1, ind);
    child(subExp2, ind);
    child(subExp3, ind);
}

void Const::printx(int ind) const {
    alignStream(LOG_STREAM(),ind) << operStrings[op] << "\n";
    switch (op) {
    case opIntConst:
        LOG_STREAM() << u.i;
        break;
    case opStrConst:
        LOG_STREAM() << "\"" << strin << "\"";
        break;
    case opFltConst:
        LOG_STREAM() << u.d;
        break;
    case opFuncConst:
        LOG_STREAM() << qPrintable(u.pp->getName());
        break;
    default:
        LOG_STREAM() << "?" << (int)op << "?";
    }
    if (conscript)
        LOG_STREAM() << " \\" << conscript << "\\";
    LOG_STREAM() << '\n';
    LOG_STREAM().flush();
}

void TypeVal::printx(int ind) const {
    alignStream(LOG_STREAM(),ind) << operStrings[op] << " ";
    LOG_STREAM() << val->getCtype() << "\n";
    LOG_STREAM().flush();
}

void TypedExp::printx(int ind) const {
    alignStream(LOG_STREAM(),ind) << operStrings[op] << " ";
    LOG_STREAM() << type->getCtype() << "\n";
    LOG_STREAM().flush();
    child(subExp1, ind);
}

void Terminal::printx(int ind) const {
    alignStream(LOG_STREAM(),ind) << operStrings[op] << "\n";
    LOG_STREAM().flush();
}

void RefExp::printx(int ind) const {
    alignStream(LOG_STREAM(),ind) << operStrings[op] << "\n";
    LOG_STREAM() << "{";
    if (def == nullptr)
        LOG_STREAM() << "nullptr";
    else
        LOG_STREAM() << ADDRESS::host_ptr(def) << "=" << def->getNumber();
    LOG_STREAM() << "}\n";
    LOG_STREAM().flush();
    child(subExp1, ind);
}

QString Exp::getAnyStrConst() {
    Exp *e = this;
    if (op == opAddrOf) {
        e = ((Location *)this)->getSubExp1();
        if (e->op == opSubscript)
            e = ((Unary *)e)->getSubExp1();
        if (e->op == opMemOf)
            e = ((Location *)e)->getSubExp1();
    }
    if (e->op != opStrConst)
        return QString::null;
    return ((Const *)e)->getStr();
}

// Find the locations used by this expression. Use the UsedLocsFinder visitor class
// If memOnly is true, only look inside m[...]
void Exp::addUsedLocs(LocationSet &used, bool memOnly) {
    UsedLocsFinder ulf(used, memOnly);
    accept(&ulf);
}

// Subscript any occurrences of e with e{def} in this expression
Exp *Exp::expSubscriptVar(Exp *e, Instruction *def) {
    ExpSubscripter es(e, def);
    return accept(&es);
}

// Subscript any occurrences of e with e{-} in this expression Note: subscript with nullptr, not implicit assignments as
// above
Exp *Exp::expSubscriptValNull(Exp *e) { return expSubscriptVar(e, nullptr); }

// Subscript all locations in this expression with their implicit assignments
Exp *Exp::expSubscriptAllNull(/*Cfg* cfg*/) {
    return expSubscriptVar(new Terminal(opWild), nullptr /* was nullptr, nullptr, cfg */);
}

Location *Location::local(const QString &nam, UserProc *p) { return new Location(opLocal, new Const(nam), p); }

// Don't put in exp.h, as this would require statement.h including before exp.h
bool RefExp::isImplicitDef() { return def == nullptr || def->getKind() == STMT_IMPASSIGN; }

Exp *Exp::bypass() {
    CallBypasser cb(nullptr);
    return accept(&cb);
}

void Exp::bypassComp() {
    if (op != opMemOf)
        return;
    ((Location *)this)->setSubExp1(((Location *)this)->getSubExp1()->bypass());
}

int Exp::getComplexityDepth(UserProc *proc) {
    ComplexityFinder cf(proc);
    accept(&cf);
    return cf.getDepth();
}

int Exp::getMemDepth() {
    MemDepthFinder mdf;
    accept(&mdf);
    return mdf.getDepth();
}

// Propagate all possible statements to this expression
Exp *Exp::propagateAll() {
    ExpPropagator ep;
    return accept(&ep);
}

// Propagate all possible statements to this expression, and repeat until there is no further change
Exp *Exp::propagateAllRpt(bool &changed) {
    ExpPropagator ep;
    changed = false;
    Exp *ret = this;
    while (true) {
        ep.clearChanged(); // Want to know if changed this *last* accept()
        ret = ret->accept(&ep);
        if (ep.isChanged())
            changed = true;
        else
            break;
    }
    return ret;
}

bool Exp::containsFlags() {
    FlagsFinder ff;
    accept(&ff);
    return ff.isFound();
}

// Check if this expression contains a bare memof (no subscripts) or one that has no symbol (i.e. is not a local
// variable or a parameter)
bool Exp::containsBadMemof(UserProc *proc) {
    BadMemofFinder bmf(proc);
    accept(&bmf);
    return bmf.isFound();
}

// No longer used
bool Exp::containsMemof(UserProc *proc) {
    ExpHasMemofTester ehmt(proc);
    accept(&ehmt);
    return ehmt.getResult();
}

