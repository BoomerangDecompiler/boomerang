/*
 * Copyright (C) 2002, Mike Van Emmerik and Trent Waddington
 */
/*==============================================================================
 * FILE:       exp.cc
 * OVERVIEW:   Implementation of the Exp and related classes.
 *============================================================================*/
/*
 * $Revision$
 * 05 Apr 02 - Mike: Created
 * 05 Apr 02 - Mike: Added copy constructors; was crashing under Linux
 * 08 Apr 02 - Mike: Added Terminal subclass
 * 12 Apr 02 - Mike: IDX -> OPER
 * 14 Apr 02 - Mike: search and replace functions take Exp*, was Exp&
 * 27 Apr 02 - Mike: decideType moved here from sslinst.cc
 * 10 May 02 - Mike: Added refSubExp1 etc
 * 13 May 02 - Mike: Added many more cases to print functions
 * 23 May 02 - Mike: Added error messages before several asserts
 * 02 Jun 02 - Mike: Fixed a nasty bug in Unary::polySimplify() where a member
 *              variable was used after "this" had been ;//deleted
 * 10 Jul 02 - Mike: Added simplifyAddr() methods
 * 16 Jul 02 - Mike: Fixed memory issues with operator==
 * ?? Nov 02 - Mike: Added Exp::prints (great for debugging)
 * 26 Nov 02 - Mike: Quelched some warnings; fixed an error in Assign copy
 *              constructor
 * 03 Dec 02 - Mike: Fixed simplification of exp AND -1 (was exp AND +1)
 * 09 Dec 02 - Mike: Print succ()
 * 03 Feb 03 - Mike: Mods for cached dataflow
 * 25 Mar 03 - Mike: Print new operators (opWildIntConst, etc)
 * 10 Jun 03 - Mike: Swapped simplification of a - K -> a + -K
 * 16 Jun 03 - Mike: Binary::simplifyArith simplifies subexpressions first;
 *              returns a - K now (was a + -K)
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <numeric>      // For accumulate
#include <algorithm>    // For std::max()
#include <map>          // In decideType()
#include <sstream>      // Need gcc 3.0 or better
#include "types.h"
#include "statement.h"
#include "cfg.h"
#include "exp.h"
#include "register.h"
#include "rtl.h"        // E.g. class ParamEntry in decideType()
#include "proc.h"
#include "prog.h"
#include "operstrings.h"// Defines a large array of strings for the
                        // createDotFile functions. Needs -I. to find it
#include "util.h"
#include "boomerang.h"

/*==============================================================================
 * FUNCTION:        Const::Const etc
 * OVERVIEW:        Constructors
 * PARAMETERS:      As required
 * RETURNS:         <nothing>
 *============================================================================*/

// Derived class constructors

Const::Const(int i)     : Exp(opIntConst)   {u.i = i;}
Const::Const(long long ll): Exp(opLongConst){u.ll= ll;}
Const::Const(double d)  : Exp(opFltConst)   {u.d = d;}
Const::Const(char* p)   : Exp(opStrConst)   {u.p = p;}
// Note: need something special for opCodeAddr
Const::Const(ADDRESS a)     : Exp(opIntConst)   {u.a = a;}

// Copy constructor
Const::Const(Const& o) : Exp(o.op) {u = o.u;}

Terminal::Terminal(OPER op) : Exp(op) {}
Terminal::Terminal(Terminal& o) : Exp(o.op) {}      // Copy constructor

Unary::Unary(OPER op)
    : Exp(op) {
    subExp1 = 0;        // Initialise the pointer
}
Unary::Unary(OPER op, Exp* e)
    : Exp(op) {
    subExp1 = e;        // Initialise the pointer
}
Unary::Unary(Unary& o)
    : Exp(o.op) {
    subExp1 = o.subExp1->clone();
}

Binary::Binary(OPER op)
    : Unary(op) {
    subExp2 = 0;        // Initialise the 2nd pointer. The first
                        // pointer is initialised in the Unary constructor
}
Binary::Binary(OPER op, Exp* e1, Exp* e2)
    : Unary(op, e1)
{
    subExp2 = e2;       // Initialise the 2nd pointer
}
Binary::Binary(Binary& o)
    : Unary(op)
{
    setSubExp1( subExp1->clone());
    subExp2 = o.subExp2->clone();
}

Ternary::Ternary(OPER op)
    : Binary(op)
{
    subExp3 = 0;
}
Ternary::Ternary(OPER op, Exp* e1, Exp* e2, Exp* e3)
    : Binary(op, e1, e2)
{
    subExp3 = e3;
}
Ternary::Ternary(Ternary& o)
    : Binary(o.op)
{
    subExp1 = o.subExp1->clone();
    subExp2 = o.subExp2->clone();
    subExp3 = o.subExp3->clone();
}

TypedExp::TypedExp() : Unary(opTypedExp), type(NULL) { }
TypedExp::TypedExp(Exp* e1) : Unary(opTypedExp, e1), type(NULL) { }
TypedExp::TypedExp(Type* ty, Exp* e1) : Unary(opTypedExp, e1),
    type(ty) { }
TypedExp::TypedExp(TypedExp& o) : Unary(opTypedExp)
{
    subExp1 = o.subExp1->clone();
    type = o.type->clone();
}

FlagDef::FlagDef(Exp* params, RTL* rtl)
    : Unary(opFlagDef, params), rtl(rtl) {}

RefExp::RefExp(Exp* e, Statement* d) : Unary(opSubscript, e), def(d) {
}

PhiExp::PhiExp(Exp* e, Statement* d) : Unary(opPhi, e)
{   stmtVec.putAt(0, d);
}

PhiExp::PhiExp(PhiExp& o) : Unary(opPhi, subExp1)
{   stmtVec = o.stmtVec;      // No need to clone: statements never move
}

TypeVal::TypeVal(Type* ty) : Terminal(opTypeVal), val(ty)
{ }

/*==============================================================================
 * FUNCTION:        Unary::~Unary etc
 * OVERVIEW:        Destructors.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
Unary::~Unary() {
    // Remember to ;//delete all children
    if (subExp1 != 0) ;//delete subExp1;
}
Binary::~Binary() {
    if (subExp2 != 0) ;//delete subExp2;
    // Note that the first pointer is destructed in the Exp1 destructor
}
Ternary::~Ternary() {
    if (subExp3 != 0) ;//delete subExp3;
}
FlagDef::~FlagDef() {
    ;//delete rtl;
}
TypeVal::~TypeVal() {
    ;//delete val;
}

/*==============================================================================
 * FUNCTION:        Unary::setSubExp1 etc
 * OVERVIEW:        Set requested subexpression; 1 is first
 * PARAMETERS:      Pointer to subexpression to set
 * NOTE:            If an expression already exists, it is ;//deleted
 * RETURNS:         <nothing>
 *============================================================================*/
void Unary::setSubExp1(Exp* e) {
    if (subExp1 != 0) ;//delete subExp1;
    subExp1 = e;
}
void Binary::setSubExp2(Exp* e) {
    if (subExp2 != 0) ;//delete subExp2;
    subExp2 = e;
}
void Ternary::setSubExp3(Exp* e) {
    if (subExp3 != 0) ;//delete subExp3;
    subExp3 = e;
}
/*==============================================================================
 * FUNCTION:        Unary::getSubExp1 etc
 * OVERVIEW:        Get subexpression
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to the requested subexpression
 *============================================================================*/
Exp* Unary::getSubExp1() {
    return subExp1;
}
Exp*& Unary::refSubExp1() {
    return subExp1;
}
Exp* Binary::getSubExp2() {
    return subExp2;
}
Exp*& Binary::refSubExp2() {
    return subExp2;
}
Exp* Ternary::getSubExp3() {
    return subExp3;
}
Exp*& Ternary::refSubExp3() {
    return subExp3;
}

// This to satisfy the compiler (never gets called!)
Exp* dummy;
Exp*& Exp::refSubExp1() {return dummy;}
Exp*& Exp::refSubExp2() {return dummy;}
Exp*& Exp::refSubExp3() {return dummy;}

Type* TypedExp::getType()
{
    return type;
}
void TypedExp::setType(Type* ty)
{
    if (type) ;//delete type;
    type = ty;
}

/*==============================================================================
 * FUNCTION:        Binary::commute
 * OVERVIEW:        Swap the two subexpressions
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Binary::commute() {
    Exp* t = subExp1;
    subExp1 = subExp2;
    subExp2 = t;
}
/*==============================================================================
 * FUNCTION:        Unary::becomeSubExp1() etc
 * OVERVIEW:        "Become" the subexpression. This is used to more efficiently
 *                    perform simplifications, which could otherwise require
 *                    the copying then deleting of large subtrees
 *                    Example: 0 + B -> B where B is a large subexpression
 * NOTE:            This (enclosing) expression is ;//deleted
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to the requested subexpression
 *============================================================================*/
Exp* Unary::becomeSubExp1() {
    Exp* res = subExp1;
    subExp1 = 0;            // Change pointer to become NULL
    ;//delete this;            // Suicide!
    return res;
}
Exp* Binary::becomeSubExp2() {
    Exp* res = subExp2;
    subExp2 = 0;            // Change pointer to become NULL
    ;//delete this;            // Suicide!
    return res;
}
Exp* Ternary::becomeSubExp3() {
    Exp* res = subExp3;
    subExp3 = 0;            // Change pointer to become NULL
    ;//delete this;            // Suicide!
    return res;
}
/*==============================================================================
 * FUNCTION:        Const::clone etc
 * OVERVIEW:        Virtual function to make a clone of myself, i.e. to create
 *                   a new Exp with the same contents as myself, but not sharing
 *                   any memory. Deleting the clone will not affect this object.
 *                   Pointers to subexpressions are not copied, but also cloned.
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to cloned object
 *============================================================================*/
Exp* Const::clone() {
    return new Const(*this);
}
Exp* Terminal::clone() {
    return new Terminal(*this);
}
Exp* Unary::clone() {
    Unary* c = new Unary(op);
    c->subExp1 = subExp1->clone();
    return c;
}
Exp* Binary::clone() {
    Binary* c = new Binary(op);
    c->subExp1 = subExp1->clone();
    c->subExp2 = subExp2->clone();
    return c;
}

Exp* Ternary::clone() {
    Ternary* c = new Ternary(op);
    c->subExp1 = subExp1->clone();
    c->subExp2 = subExp2->clone();
    c->subExp3 = subExp3->clone();
    return c;
}
Exp* TypedExp::clone() {
    TypedExp* c = new TypedExp(type, subExp1->clone());
    return c;
}
Exp* PhiExp::clone() {
    PhiExp* c = new PhiExp(subExp1->clone());
    c->stmtVec = stmtVec;
    return c;
}
Exp* RefExp::clone() {
    RefExp* c = new RefExp(subExp1->clone(), def);
    return c;
}

Exp* TypeVal::clone() {
    TypeVal* c = new TypeVal(val->clone());
    return c;
}


/*==============================================================================
 * FUNCTION:        Const::operator==() etc
 * OVERVIEW:        Virtual function to compare myself for equality with
 *                  another Exp
 * PARAMETERS:      Ref to other Exp
 * RETURNS:         True if equal
 *============================================================================*/
bool Const::operator==(const Exp& o) const {
    // Note: the casts of o to Const& are needed, else op is protected! Duh.
    if (((Const&)o).op == opWild) return true;
    if (((Const&)o).op == opWildIntConst && op == opIntConst) return true;
    if (op != ((Const&)o).op) return false;
    switch (op) {
        case opIntConst: return u.i == ((Const&)o).u.i;
        case opFltConst: return u.d == ((Const&)o).u.d;
        case opStrConst: return (strcmp(u.p, ((Const&)o).u.p) == 0);
        default: LOG << "Operator== invalid operator " << operStrings[op]
                   << "\n";
                 assert(0);
    }
    return false;
}
bool Unary::operator==(const Exp& o) const {
    //if (op == opWild) return true;
    if (((Unary&)o).op == opWild) return true;
    if (((Unary&)o).op == opWildRegOf && op == opRegOf) return true;
    if (((Unary&)o).op == opWildMemOf && op == opMemOf) return true;
    if (((Unary&)o).op == opWildAddrOf && op == opAddrOf) return true;
    if (op != ((Unary&)o).op) return false;
    return *subExp1 == *((Unary&)o).getSubExp1();
}
bool Binary::operator==(const Exp& o) const {
    //if (op == opWild) return true;
    if (((Binary&)o).op == opWild) return true;
    if (op != ((Binary&)o).op)     return false;
    if (!( *subExp1 == *((Binary&)o).getSubExp1())) return false;
    return *subExp2 == *((Binary&)o).getSubExp2();
}

bool Ternary::operator==(const Exp& o) const {
    //if (op == opWild) return true;
    if (((Ternary&)o).op == opWild) return true;
    if (op != ((Ternary&)o).op) return false;
    if (!( *subExp1 == *((Ternary&)o).getSubExp1())) return false;
    if (!( *subExp2 == *((Ternary&)o).getSubExp2())) return false;
    return *subExp3 == *((Ternary&)o).getSubExp3();
}
bool Terminal::operator==(const Exp& o) const {
    if (op == opWildIntConst) return ((Terminal&)o).op == opIntConst;
    if (op == opWildMemOf)    return ((Terminal&)o).op == opMemOf;
    if (op == opWildRegOf)    return ((Terminal&)o).op == opRegOf;
    if (op == opWildAddrOf)   return ((Terminal&)o).op == opAddrOf;
    return ((op == opWild) ||           // Wild matches anything
            (((Terminal&)o).op == opWild) ||
            (op == ((Terminal&)o).op));
}
bool TypedExp::operator==(const Exp& o) const {
    //if (op == opWild) return true;
    if (((TypedExp&)o).op == opWild) return true;
    if (((TypedExp&)o).op != opTypedExp) return false;
    // This is the strict type version
    if (*type != *((TypedExp&)o).type) return false;
    return *((Unary*)this)->getSubExp1() == *((Unary&)o).getSubExp1();
}

bool RefExp::operator==(const Exp& o) const {
    if (op == opWild) return true;
    if (((RefExp&)o).op == opWild) return true;
    if (((RefExp&)o).op != opSubscript) return false;
    if (!( *subExp1 == *((RefExp&)o).subExp1)) return false;
    // Allow a def of (Statement*)-1 as a wild card
    if ((int)def == -1) return true;
    if ((int)((RefExp&)o).def == -1) return true;
    return def == ((RefExp&)o).def;
}

bool PhiExp::operator==(const Exp& o) const {
    if (op == opWild) return true;
    if (((PhiExp&)o).op == opWild) return true;
    if (((PhiExp&)o).op != opPhi) return false;
    if (!( *subExp1 == *((PhiExp&)o).subExp1)) return false;
    return stmtVec == ((PhiExp&)o).stmtVec;
}

bool TypeVal::operator==(const Exp& o) const {
    if (op == opWild) return true;
    if (((TypeVal&)o).op == opWild) return true;
    if (((TypeVal&)o).op != opTypeVal) return false;
    return *val == *((TypeVal&)o).val;
}

// Compare, ignoring subscripts
bool Exp::operator*=(const Exp& o) const {
    const Exp* one = this;
    if (op == opSubscript)
        one = ((RefExp*)this)->getSubExp1();
    Exp* two = const_cast<Exp*>(&o);
    if (two->isSubscript())
        two = ((RefExp*)two)->getSubExp1();
    return *one == *two;
}

/*==============================================================================
 * FUNCTION:        Const::operator%=() etc
 * OVERVIEW:        Virtual function to compare myself for equality with
 *                  another Exp, *ignoring type*
 * NOTE:            This is overridden for TypedExp only
 * PARAMETERS:      Ref to other Exp
 * RETURNS:         True if equal
 *============================================================================*/
bool Exp::operator%=(const Exp& o) const {
    const Exp* typeless = &o;
    if (o.op == opTypedExp)
        typeless = ((Unary&)o).getSubExp1();
    return *this == *typeless;
}
bool TypedExp::operator%=(const Exp& o) const {
    const Exp* typeless = &o;
    if (o.getOper() == opTypedExp)
        typeless = ((Unary&)o).getSubExp1();
    return *((Unary*)this)->getSubExp1() == *typeless;
} 

// As above, but sign insensitive (otherwise, type sensitive)
bool Exp::operator-=(const Exp& o) const {
    const Exp* typeless = &o;
    if (o.op == opTypedExp)
        typeless = ((Unary&)o).getSubExp1();
    return *this == *typeless;
}
bool TypedExp::operator-=(const Exp& o) const {
    const Exp* typeless = &o;
    if (o.getOper() == opTypedExp) {
        typeless = ((Unary&)o).getSubExp1();
        // Both exps are typed. Do a sign insensitive type comparison
        //if (*type -= *((TypedExp&)o).type) return false;
    }
    return *((Unary*)this)->getSubExp1() == *typeless;
} 

/*==============================================================================
 * FUNCTION:        Const::operator<() etc
 * OVERVIEW:        Virtual function to compare myself with another Exp
 * NOTE:            The test for a wildcard is only with this object, not
 *                    the other object (o). So when searching and there could
 *                    be wildcards, use search == *this not *this == search
 * PARAMETERS:      Ref to other Exp
 * RETURNS:         True if equal
 *============================================================================*/
bool Const::operator< (const Exp& o) const {
    if (op < o.getOper()) return true;
    if (op > o.getOper()) return false;
    switch (op) {
        case opIntConst:
            return u.i < ((Const&)o).u.i;
        case opFltConst:
            return u.d < ((Const&)o).u.d;
        case opStrConst:
            return strcmp(u.p, ((Const&)o).u.p) < 0;
        default: LOG << "Operator< invalid operator " << operStrings[op]
                   << "\n";
                assert(0);
    }
    return false;
}
bool Terminal::operator< (const Exp& o) const {
    return (op < o.getOper());
}

bool Unary::operator< (const Exp& o) const {
    if (op < o.getOper()) return true;
    if (op > o.getOper()) return false;
    return *subExp1 < *((Unary&)o).getSubExp1();
}

bool Binary::operator< (const Exp& o) const {
    if (op < o.getOper()) return true;
    if (op > o.getOper()) return false;
    if (*subExp1 < *((Binary&)o).getSubExp1()) return true;
    if (*((Binary&)o).getSubExp1() < *subExp1) return false;
    return *subExp2 < *((Binary&)o).getSubExp2();
}

bool Ternary::operator< (const Exp& o) const {
    if (op < o.getOper()) return true;
    if (op > o.getOper()) return false;
    if (*subExp1 < *((Ternary&)o).getSubExp1()) return true;
    if (*((Ternary&)o).getSubExp1() < *subExp1) return false;
    if (*subExp2 < *((Ternary&)o).getSubExp2()) return true;
    if (*((Ternary&)o).getSubExp2() < *subExp2) return false;
    return *subExp3 < *((Ternary&)o).getSubExp3();
}

bool TypedExp::operator<< (const Exp& o) const {        // Type insensitive
    if (op < o.getOper()) return true;
    if (op > o.getOper()) return false;
    return *subExp1 << *((Unary&)o).getSubExp1();
}

bool TypedExp::operator<  (const Exp& o) const {        // Type sensitive
    if (op < o.getOper()) return true;
    if (op > o.getOper()) return false;
    if (*type < *((TypedExp&)o).type) return true;
    if (*((TypedExp&)o).type < *type) return false;
    return *subExp1 < *((Unary&)o).getSubExp1();
}

bool RefExp::operator< (const Exp& o) const {
    if (opSubscript < o.getOper()) return true;
    if (opSubscript > o.getOper()) return false;
    if (*subExp1 < *((Unary&)o).getSubExp1()) return true;
    if (*((Unary&)o).getSubExp1() < *subExp1) return false;
    // Allow a wildcard def to match any
    if (def == (Statement*)-1) return false;    // Not less (equal)
    if (((RefExp&)o).def == (Statement*)-1) return false;
    return def < ((RefExp&)o).def;
}

bool PhiExp::operator< (const Exp& o) const {
    if (opPhi < o.getOper()) return true;
    if (opPhi > o.getOper()) return false;
    if (*subExp1 < *((Unary&)o).getSubExp1()) return true;
    if (*((Unary&)o).getSubExp1() < *subExp1) return false;
    return stmtVec < ((PhiExp&)o).stmtVec;
}

bool TypeVal::operator< (const Exp& o) const {
    if (opTypeVal < o.getOper()) return true;
    if (opTypeVal > o.getOper()) return false;
    return *val < *((TypeVal&)o).val;
}




/*==============================================================================
 * FUNCTION:        Const::print etc
 * OVERVIEW:        "Print" in infix notation the expression to a stream
 *                  Mainly for debugging, or maybe some low level windows
 * PARAMETERS:      Ref to an output stream
 * RETURNS:         <nothing>
 *============================================================================*/

//  //  //  //
//  Const   //
//  //  //  //
void Const::print(std::ostream& os, bool withUses) {
    switch (op) {
        case opIntConst:
            os << std::dec << u.i;
            break;
        case opFltConst:
            char buf[64];
            sprintf(buf, "%g", u.d);
            os << buf;
            break;
        case opStrConst:
            os << "\"" << u.p << "\"";
            break;
        default:
            LOG << "Const::print invalid operator " << operStrings[op] <<
              "\n";
            assert(0);
    }
}

void Const::printNoQuotes(std::ostream& os, bool withUses) {
    if (op == opStrConst)
        os << u.p;
    else
        print(os, withUses);
}

//  //  //  //
//  Binary  //
//  //  //  //
void Binary::printr(std::ostream& os, bool withUses) {
    // The "r" is for recursive: the idea is that we don't want parentheses at
    // the outer level, but a subexpression (recursed from a higher level), we
    // want the parens (at least for standard infix operators)
    switch (op) {
        case opSize:
        case opList:        // Otherwise, you get (a, (b, (c, d)))
            // There may be others
            // These are the noparen cases
            print(os, withUses); return;
        default:
            break;
    }
    // Normal case: we want the parens
    // std::ostream::operator<< uses print(), which does not have the parens
    os << "(" << this << ")";
}

void Binary::print(std::ostream& os, bool withUses) {
    Exp* p1; Exp* p2;
    p1 = ((Binary*)this)->getSubExp1();
    p2 = ((Binary*)this)->getSubExp2();
    // Special cases
    switch (op) {
        case opSize:
            // *size* is printed after the expression
            p2->printr(os, withUses); os << "*"; p1->printr(os, withUses);
            os << "*";
            assert(false); // thought this was deprecated.
            return;
        case opFlagCall:
            // The name of the flag function (e.g. ADDFLAGS) should be enough
            ((Const*)p1)->printNoQuotes(os, withUses);
            os << "( "; p2->printr(os, withUses); os << " )";
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
            // Because "," is the lowest precedence operator, we don't need
            // printr here. Also, same as UQBT, so easier to test
            p1->print(os, withUses);
            if (!p2->isNil())
                os << ", "; 
            p2->print(os, withUses);
            return;

        case opMemberAccess:
            p1->print(os, withUses);
            os << ".";
            ((Const*)p2)->printNoQuotes(os, withUses);
            return;

        default:
            break;
    }

    // Ordinary infix operators. Emit parens around the binary
    if (p1 == NULL)
        os << "<NULL>";
    else
        p1->printr(os, withUses);
    switch (op) {
        case opPlus:    os << " + ";  break;
        case opMinus:   os << " - ";  break;
        case opMult:    os << " * ";  break;
        case opMults:   os << " *! "; break;
        case opDiv:     os << " / ";  break;
        case opDivs:    os << " /! "; break;
        case opMod:     os << " % ";  break;
        case opMods:    os << " %! "; break;
        case opFPlus:   os << " +f "; break;
        case opFMinus:  os << " -f "; break;
        case opFMult:   os << " *f "; break;
        case opFDiv:    os << " /f "; break;
        case opAnd:     os << " and ";break;
        case opOr:      os << " or "; break;
        case opBitAnd:  os << " & ";  break;
        case opBitOr :  os << " | ";  break;
        case opBitXor:  os << " ^ ";  break;
        case opEquals:  os << " = ";  break;
        case opNotEqual:os << " ~= "; break;
        case opLess:    os << " < ";  break;
        case opGtr:     os << " > ";  break;
        case opLessEq:  os << " <= "; break;
        case opGtrEq:   os << " >= "; break;
        case opLessUns: os << " <u "; break;
        case opGtrUns:  os << " >u "; break;
        case opLessEqUns:os<< " <=u ";break;
        case opGtrEqUns:os << " >=u ";break;
        case opUpper:   os << " GT "; break;
        case opLower:   os << " LT "; break;
        case opShiftL:  os << " << "; break;
        case opShiftR:  os << " >> "; break;
        case opShiftRA: os << " >>A "; break;
        case opRotateL: os << " rl "; break;
        case opRotateR: os << " rr "; break;
        case opRotateLC:os << " rlc "; break;
        case opRotateRC:os << " rrc "; break;

        default:
            LOG << "Binary::print invalid operator " << operStrings[op]
              << "\n";
            assert(0);
    }

    if (p2 == NULL)
        os << "<NULL>";
    else
        p2->printr(os, withUses);

}


//  //  //  //  //
//   Terminal   //
//  //  //  //  //
void Terminal::print(std::ostream& os, bool withUses) {
    switch (op) {
        case opPC:      os << "%pc";   break;
        case opFlags:   os << "%flags"; break;
        case opCF:      os << "%CF";   break;
        case opZF:      os << "%ZF";   break;
        case opOF:      os << "%OF";   break;
        case opNF:      os << "%NF";   break;
        case opDF:      os << "%DF";   break;
        case opAFP:     os << "%afp";  break;
        case opAGP:     os << "%agp";  break;
        case opWild:    os << "WILD";  break;
        case opAnull:   os << "%anul"; break;
        case opFpush:   os << "FPUSH"; break;
        case opFpop:    os << "FPOP";  break;
        case opPhi:     os << "phi"; break;
        case opWildMemOf:os<< "m[WILD]"; break;
        case opWildRegOf:os<< "r[WILD]"; break;
        case opWildAddrOf:os<< "a[WILD]"; break;
        case opWildIntConst:os<<"WILDINT"; break;
        case opNil:     break;
        case opTrue:    os << "true"; break;
        case opFalse:   os << "false"; break;
        default:
            LOG << "Terminal::print invalid operator " << operStrings[op]
              << "\n";
            assert(0);
    }
}

//  //  //  //
//   Unary  //
//  //  //  //
void Unary::print(std::ostream& os, bool withUses) {
    Exp* p1 = ((Unary*)this)->getSubExp1();
    switch (op) {
        //  //  //  //  //  //  //
        //  x[ subexpression ]  //
        //  //  //  //  //  //  //
        case opRegOf:
            // Make a special case for the very common case of r[intConst]
            if (p1->isIntConst()) {
                os << "r" << std::dec << ((Const*)p1)->getInt();
                break;
            } else if (p1->isTemp()) {
                // Just print the temp {
                p1->print(os, withUses);
                break;
            }
            // Else fall through
        case opMemOf: case opAddrOf:  case opVar:
            switch (op) {
                case opRegOf: os << "r["; break;    // e.g. r[r2]
                case opMemOf: os << "m["; break;
                case opAddrOf:os << "a["; break;
                case opVar:   os << "v["; break;
                default: break;     // Suppress compiler warning
            }
            if (op == opVar) ((Const*)p1)->printNoQuotes(os, withUses);
            // Use print, not printr, because this is effectively the top
            // level again (because the [] act as parentheses)
            else p1->print(os, withUses);
            os << "]";
            break;

        //  //  //  //  //  //  //
        //    Unary operators   //
        //  //  //  //  //  //  //

        case opNot:     case opLNot:    case opNeg:
                 if (op == opNot)  os << "~";
            else if (op == opLNot) os << "L~";
            else                   os << "-";
            p1->printr(os, withUses);
            return;

        case opSignExt:
            p1->printr(os, withUses);
            os << "!";          // Operator after expression
            return;

        //  //  //  //  //  //  //  //
        //  Function-like operators //
        //  //  //  //  //  //  //  //

        case opSQRTs: case opSQRTd: case opSQRTq:
        case opSqrt: case opSin: case opCos:
        case opTan: case opArcTan: case opLog2:
        case opLog10: case opLoge: case opMachFtr:
        case opSuccessor:
            switch (op) {
                case opSQRTs: os << "SQRTs("; break;
                case opSQRTd: os << "SQRTd("; break;
                case opSQRTq: os << "SQRTq("; break;
                case opSqrt:  os << "sqrt("; break;
                case opSin:   os << "sin("; break;
                case opCos:   os << "cos("; break;
                case opTan:   os << "tan("; break;
                case opArcTan:os << "arctan("; break;
                case opLog2:  os << "log2("; break;
                case opLog10: os << "log10("; break;
                case opLoge:  os << "loge("; break;
                case opExecute:os<< "execute("; break;
                case opMachFtr:os << "machine("; break;
                case opSuccessor: os << "succ("; break;
                default: break;         // For warning
            }
            p1->printr(os, withUses);
            os << ")";
            return;

        //  Misc    //
        case opSgnEx:      // Different because the operator appears last
            p1->printr(os, withUses);
            os << "! ";
            return;
        case opTemp:
            // Temp: just print the string, no quotes
        case opGlobal:
        case opLocal:
        case opParam:
            // Print a more concise form than param["foo"] (just foo)
            ((Const*)p1)->printNoQuotes(os, withUses);
            return;
        case opPhi:
            os << "phi(";
            p1->print(os, withUses);
            os << ")";
            return;
        case opTypeOf:
            os << "T";
            p1->print(os, withUses);
            return;
        default:
            LOG << "Unary::print invalid operator " << operStrings[op] <<
              "\n";
            assert(0);
    }
}

//  //  //  //
//  Ternary //
//  //  //  //
void Ternary::printr(std::ostream& os, bool withUses) {
    // The function-like operators don't need parentheses
    switch (op) {
        // The "function-like" ternaries
        case opTruncu:  case opTruncs:  case opZfill:
        case opSgnEx:   case opFsize:   case opItof:
        case opFtoi:    case opFround:  case opOpTable:
            // No paren case
            print(os, withUses); return;
        default:
            break;
    }
    // All other cases, we use the parens
    os << "(" << this << ")";
}

void Ternary::print(std::ostream& os, bool withUses) {
    Exp* p1 = ((Ternary*)this)->getSubExp1();
    Exp* p2 = ((Ternary*)this)->getSubExp2();
    Exp* p3 = ((Ternary*)this)->getSubExp3();
    switch (op) {
        // The "function-like" ternaries
        case opTruncu:  case opTruncs:  case opZfill:
        case opSgnEx:   case opFsize:   case opItof:
        case opFtoi:    case opFround:  case opOpTable:
            switch (op) {
                case opTruncu:  os << "truncu("; break;
                case opTruncs:  os << "truncs("; break;
                case opZfill:   os << "zfill("; break;
                case opSgnEx:   os << "sgnex("; break;
                case opFsize:   os << "fsize("; break;
                case opItof:    os << "itof(";  break;
                case opFtoi:    os << "ftoi(";  break;
                case opFround:  os << "fround("; break;
                case opOpTable: os << "optable("; break;
                default: break;         // For warning
            }
            // Use print not printr here, since , has the lowest precendence
            // of all. Also it makes it the same as UQBT, so it's easier to test
            if (p1) p1->print(os, withUses); else os << "<NULL>"; os << ",";
            if (p2) p2->print(os, withUses); else os << "<NULL>"; os << ",";
            if (p3) p3->print(os, withUses); else os << "<NULL>"; os << ")";
            return;
        default:
            break;
    }
    // Else must be ?: or @ (traditional ternary operators)
    if (p1) p1->printr(os, withUses); else os << "<NULL>";
    if (op == opTern) {
        os << " ? ";
        if (p2) p2->printr(os, withUses); else os << "<NULL>";
        os << " : ";        // Need wide spacing here
        if (p3) p3->print(os, withUses); else os << "<NULL>";
    } 
    else if (op == opAt) {
            os << "@";
            if (p2) p2->printr(os, withUses); else os << "NULL>";
            os << ":";
            if (p3) p3->printr(os, withUses); else os << "NULL>";
    } else {
        LOG << "Ternary::print invalid operator " << operStrings[op] <<
          "\n";
        assert(0);
    }
}

//  //  //  //
// TypedExp //
//  //  //  //
void TypedExp::print(std::ostream& os, bool withUses) {
    os << "*" << std::dec << type->getSize() << "* ";
    Exp* p1 = ((Ternary*)this)->getSubExp1();
    p1->print(os, withUses);
}


//  //  //  //
//  RefExp  //
//  //  //  //
void RefExp::print(std::ostream& os, bool withUses) {
    subExp1->print(os, withUses);
    if (withUses) {
        os << "{";
        if (def == (Statement*)-1) os << "WILD";
        else if (def) def->printNum(os);
        else os << "0";
        os << "}";
    }
}

//  //  //  //
// PhiExp  //
//  //  //  //
void PhiExp::print(std::ostream& os, bool withUses) {
    os << "phi";
    if (withUses) {
        os << "{";
        stmtVec.printNums(os);
        os << "}";
    }
}

//  //  //  //
// TypeVal  //
//  //  //  //
void TypeVal::print(std::ostream& os, bool withUses) {
    os << "<" << val->getCtype() << ">";
}

/*==============================================================================
 * FUNCTION:        Exp::prints
 * OVERVIEW:        Print to a static string (for debugging)
 * PARAMETERS:      <none>
 * RETURNS:         Address of the static buffer
 *============================================================================*/
extern char debug_buffer[];
char* Exp::prints() {
      std::ostringstream ost;
      print(ost, true);
      strncpy(debug_buffer, ost.str().c_str(), 199);
      debug_buffer[199] = '\0';
      return debug_buffer;
}


/*==============================================================================
 * FUNCTION:        Exp::createDotFile etc
 * OVERVIEW:        Create a dotty file (use dotty to display the file;
 *                    search the web for "graphviz")
 *                  Mainly for debugging
 * PARAMETERS:      Name of the file to create
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::createDotFile(char* name) {
    std::ofstream of;
    of.open(name);
    if (!of) {
        LOG << "Could not open " << name << " to write dotty file\n";
        return;
    }
    of << "digraph Exp {\n";
    appendDotFile(of);
    of << "}";
    of.close();
}

//  //  //  //
//  Const   //
//  //  //  //
void Const::appendDotFile(std::ofstream& of) {
    // We define a unique name for each node as "e123456" if the
    // address of "this" == 0x123456
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    of << operStrings[op] << "\\n0x" << std::hex << (int)this << " | ";
    switch (op) {
        case opIntConst:  of << std::dec << u.i; break;
        case opFltConst:  of << u.d; break;
        case opStrConst:  of << "\\\"" << u.p << "\\\""; break;
        case opCodeAddr:  of << "0x" << std::hex << u.a; break;
        default:
            break;
    }
    of << " }\"];\n";
}

//  //  //  //
// Terminal //
//  //  //  //
void Terminal::appendDotFile(std::ofstream& of) {
    of << "e" << std::hex << (int)this << " [shape=parallelogram,label=\"";
    if (op == opWild)
        // Note: value is -1, so can't index array
        of << "WILD";
    else
        of << operStrings[op];
    of << "\\n0x" << std::hex << (int)this;
    of << "\"];\n";
}

//  //  //  //
//  Unary   //
//  //  //  //
void Unary::appendDotFile(std::ofstream& of) {
    // First a node for this Unary object
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    // The (int) cast is to print the address, not the expression!
    of << operStrings[op] << "\\n0x" << std::hex << (int)this << " | ";
    of << "<p1>";
    of << " }\"];\n";

    // Now recurse to the subexpression.
    subExp1->appendDotFile(of);

    // Finally an edge for the subexpression
    of << "e" << std::hex << (int)this << "->e" << (int)subExp1 << ";\n";
}

//  //  //  //
//  Binary  //
//  //  //  //
void Binary::appendDotFile(std::ofstream& of) {
    // First a node for this Binary object
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    of << operStrings[op] << "\\n0x" << std::hex << (int)this << " | ";
    of << "{<p1> | <p2>}";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    subExp2->appendDotFile(of);
    // Now an edge for each subexpression
    of << "e" << std::hex << (int)this << ":p1->e" << (int)subExp1 << ";\n";
    of << "e" << std::hex << (int)this << ":p2->e" << (int)subExp2 << ";\n";
}

//  //  //  //
//  Ternary //
//  //  //  //
void Ternary::appendDotFile(std::ofstream& of) {
    // First a node for this Ternary object
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    of << operStrings[op] << "\\n0x" << std::hex << (int)this << " | ";
    of << "{<p1> | <p2> | <p3>}";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    subExp2->appendDotFile(of);
    subExp3->appendDotFile(of);
    // Now an edge for each subexpression
    of << "e" << std::hex << (int)this << ":p1->e" << (int)subExp1 << ";\n";
    of << "e" << std::hex << (int)this << ":p2->e" << (int)subExp2 << ";\n";
    of << "e" << std::hex << (int)this << ":p3->e" << (int)subExp3 << ";\n";
}
//  //  //  //
// TypedExp //
//  //  //  //
void TypedExp::appendDotFile(std::ofstream& of) {
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    of << "opTypedExp\\n0x" << std::hex << (int)this << " | ";
    // Just display the C type for now
    of << type->getCtype() << " | <p1>";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    of << "e" << std::hex << (int)this << ":p1->e" << (int)subExp1 << ";\n";
}

//  //  //  //
//  FlagDef //
//  //  //  //
void FlagDef::appendDotFile(std::ofstream& of) {
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    of << "opFlagDef \\n0x" << std::hex << (int)this << "| ";
    // Display the RTL as "RTL <r1> <r2>..." vertically (curly brackets)
    of << "{ RTL ";
    int n = rtl->getNumStmt();
    for (int i=0; i < n; i++)
        of << "| <r" << std::dec << i << "> ";
    of << "} | <p1> }\"];\n";
    subExp1->appendDotFile(of);
    of << "e" << std::hex << (int)this << ":p1->e" << (int)subExp1 << ";\n";
}

/*==============================================================================
 * FUNCTION:        Exp::isRegOfK
 * OVERVIEW:        Returns true if the expression is r[K] where K is int const
 * PARAMETERS:      <none>
 * RETURNS:         True if matches
 *============================================================================*/
bool Exp::isRegOfK()
{
    if (op != opRegOf) return false;
    return ((Unary*)this)->getSubExp1()->getOper() == opIntConst;
}
/*==============================================================================
 * FUNCTION:        Exp::isRegN
 * OVERVIEW:        Returns true if the expression is r[N] where N is the given
 *                    int const
 * PARAMETERS:      N: the specific register to be tested for
 * RETURNS:         True if matches
 *============================================================================*/
bool Exp::isRegN(int N)
{
    if (op != opRegOf) return false;
    Exp* sub = ((Unary*)this)->getSubExp1();
    return (sub->getOper() == opIntConst && ((Const*)sub)->getInt() == N);
}
/*==============================================================================
 * FUNCTION:        Exp::isAfpTerm
 * OVERVIEW:        Returns true if is %afp, %afp+k, %afp-k,
 *                    or a[m[<any of these]]
 * PARAMETERS:      <none>
 * RETURNS:         True if found
 *============================================================================*/
bool Exp::isAfpTerm()
{
    Exp* cur = this;
    if (op == opTypedExp)
        cur =  ((Unary*)this)->getSubExp1();
    Exp* p;
    if ((cur->getOper() == opAddrOf) &&
      ((p =  ((Unary*)cur)->getSubExp1()), p->getOper() == opMemOf))
        cur =((Unary*)p  )->getSubExp1();
        
    OPER curOp = cur->getOper();
    if (curOp == opAFP) return true;
    if ((curOp != opPlus) && (curOp != opMinus)) return false;
    // cur must be a Binary* now
    OPER subOp1 = ((Binary*)cur)->getSubExp1()->getOper();
    OPER subOp2 = ((Binary*)cur)->getSubExp2()->getOper();
    return ((subOp1 == opAFP) && (subOp2 == opIntConst));
}


/*==============================================================================
 * FUNCTION:        Exp::getVarIndex
 * OVERVIEW:        Returns the index for this var, e.g. if v[2], return 2
 * PARAMETERS:      <none>
 * RETURNS:         The index
 *============================================================================*/
int Exp::getVarIndex() {
    assert (op == opVar);
    Exp* sub = ((Unary*)this)->getSubExp1();
    return ((Const*)sub)->getInt();
}

/*==============================================================================
 * FUNCTION:        Exp::getGuard
 * OVERVIEW:        Returns a ptr to the guard exression, or 0 if none
 * PARAMETERS:      <none>
 * RETURNS:         Ptr to the guard, or 0
 *============================================================================*/
Exp* Exp::getGuard() {
    if (op == opGuard) return ((Unary*)this)->getSubExp1();
    return NULL;
}

/*==============================================================================
 * FUNCTION:        Exp::doSearch
 * OVERVIEW:        Search for the given subexpression
 * NOTE:            Caller must free the list li after use, but not the
 *                    Exp objects that they point to
 * NOTE:            If the top level expression matches, li will contain search
 * PARAMETERS:      search: ptr to Exp we are searching for 
 *                  pSrc: ref to ptr to Exp to search. Reason is that we can
 *                    then overwrite that pointer to effect a replacement
 *                  li: list of Exp** where pointers to the matches are found
 *                  once: true if not all occurrences to be found, false for all
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::doSearch(Exp* search, Exp*& pSrc, std::list<Exp**>& li,
  bool once) {
    bool compare;
    compare = (*search == *pSrc);
    if (compare) {
        li.push_back(&pSrc);                // Success
        if (once)
            return;                         // No more to do
    }
    // Either want to find all occurrences, or did not match at this level
    // Recurse into children, unless a matching opSubscript
    if (op != opSubscript || !compare)
        pSrc->doSearchChildren(search, li, once);
}

/*==============================================================================
 * FUNCTION:        Exp::doSearchChildren
 * OVERVIEW:        Search for the given subexpression in all children
 * NOTE:            Virtual function; different implementation for each
 *                  subclass of Exp
 * NOTE:            Will recurse via doSearch
 * PARAMETERS:      search: ptr to Exp we are searching for
 *                  li: list of Exp** where pointers to the matches are found
 *                  once: true if not all occurrences to be found, false for all
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::doSearchChildren(Exp* search, std::list<Exp**>& li, bool once) {
    return;         // Const and Terminal do not override this
}
void Unary::doSearchChildren(Exp* search, std::list<Exp**>& li, bool once) {
    subExp1->doSearch(search, subExp1, li, once);
}
void Binary::doSearchChildren(Exp* search, std::list<Exp**>& li, bool once) {
    getSubExp1()->doSearch(search, subExp1, li, once);
    if (once && li.size()) return;
    subExp2->doSearch(search, subExp2, li, once);
}
void Ternary::doSearchChildren(Exp* search, std::list<Exp**>& li, bool once) {
    getSubExp1()->doSearch(search, subExp1, li, once);
    if (once && li.size()) return;
    getSubExp2()->doSearch(search, subExp2, li, once);
    if (once && li.size()) return;
    subExp3->doSearch(search, subExp3, li, once);
}


/*==============================================================================
 * FUNCTION:        Exp::searchReplace
 * OVERVIEW:        Search for the given subexpression, and replace if found
 * NOTE:            If the top level expression matches, return val != this
 * PARAMETERS:      search:  ptr to Exp we are searching for
 *                  replace: ptr to Exp to replace it with
 *                  change: ref to boolean, set true if a change made
 *                  (else cleared)
 * RETURNS:         True if a change made
 *============================================================================*/
Exp* Exp::searchReplace(Exp* search, Exp* replace, bool& change)
{
    return searchReplaceAll(search, replace, change, true);
}

/*==============================================================================
 * FUNCTION:        Exp::searchReplaceAll
 * OVERVIEW:        Search for the given subexpression, and replace wherever
 *                    found
 * NOTE:            If the top level expression matches, something other than
 *                   "this" will be returned
 * NOTE:            It is possible with wildcards that in very unusual
 *                  circumstances a replacement will be made to something that
 *                  is already ;//deleted.
 * NOTE:            Replacements are cloned. Caller to ;//delete search and replace
 * PARAMETERS:      search:  ptr to ptr to Exp we are searching for
 *                  replace: ptr to Exp to replace it with
 *                  change: set true if a change made; cleared otherwise
 * RETURNS:         the result (often this, but possibly changed)
 *============================================================================*/
Exp* Exp::searchReplaceAll(Exp* search, Exp* replace, bool& change,
  bool once /* = false */ ) {
    std::list<Exp**> li;
    Exp* top = this;        // top may change; that's why we have to return it
    doSearch(search, top, li, false);
    std::list<Exp**>::iterator it;
    for (it = li.begin(); it != li.end(); it++) {
        Exp** pp = *it;
        //if (*pp) ;//delete *pp;        // Delete any existing
        *pp = replace->clone();     // Do the replacement
        if (once) {
            change = true;
            return top;
        }
    }
    change = (li.size() != 0);
    return top;
}

/*==============================================================================
 * FUNCTION:        Exp::search
 * OVERVIEW:        Search this expression for the given subexpression, and if
 *                    found, return true and return a pointer to the matched
 *                    expression in result (useful when there are wildcards,
 *                    e.g. search pattern is r[?] result is r[2].
 * PARAMETERS:      search:  ptr to Exp we are searching for
 *                  result:  ref to ptr to Exp that matched
 * RETURNS:         True if a match was found
 *============================================================================*/
bool Exp::search(Exp* search, Exp*& result)
{
    std::list<Exp**> li;
    result = 0;             // In case it fails; don't leave it unassigned
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to
    // re-use the same search routine
    Exp* top = this;
    doSearch(search, top, li, false);
    if (li.size()) {
        result = *li.front();
        return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        Exp::searchAll
 * OVERVIEW:        Search this expression for the given subexpression, and for
 *                    each found, return a pointer to the matched
 *                    expression in result
 * PARAMETERS:      search:  ptr to Exp we are searching for
 *                  results:  ref to list of Exp that matched 
 * RETURNS:         True if a match was found
 *============================================================================*/
bool Exp::searchAll(Exp* search, std::list<Exp*>& result)
{
    std::list<Exp**> li;
    result.clear();
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to
    // re-use the same search routine
    Exp* pSrc = this;
    doSearch(search, pSrc, li, false);
    std::list<Exp**>::iterator it;
    for (it = li.begin(); it != li.end(); it++) {
        // li is list of Exp**; result is list of Exp*
        result.push_back(**it);
    }
    return li.size() != 0;
}

// These simplifying functions don't really belong in class Exp, but they know
// too much about how Exps work
// They can't go into util.so, since then util.so and db.so would co-depend
// on each other for testing at least
/*==============================================================================
 * FUNCTION:        Exp::partitionTerms
 * OVERVIEW:        Takes an expression consisting on only + and - operators and
 *                  partitions its terms into positive non-integer fixed terms,
 *                  negative non-integer fixed terms and integer terms. For
 *                  example, given:
 *                     %sp + 108 + n - %sp - 92
 *                  the resulting partition will be:
 *                     positives = { %sp, n }
 *                     negatives = { %sp }
 *                     integers  = { 108, -92 }
 * NOTE:            integers is a vector so we can use the accumulate func
 * NOTE:            Expressions are NOT cloned. Therefore, do not ;//delete the
 *                    expressions in positives or negatives
 * PARAMETERS:      positives - the list of positive terms
 *                  negatives - the list of negative terms
 *                  integers - the vector of integer terms
 *                  negate - determines whether or not to negate the whole
 *                    expression, i.e. we are on the RHS of an opMinus
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::partitionTerms(std::list<Exp*>& positives, std::list<Exp*>& negatives,
  std::vector<int>& integers, bool negate) {
    Exp* p1, *p2;
    switch (op) {
        case opPlus:
            p1 = ((Binary*)this)->getSubExp1();
            p2 = ((Binary*)this)->getSubExp2();
            p1->partitionTerms(positives, negatives, integers, negate);
            p2->partitionTerms(positives, negatives, integers, negate);
            break;
        case opMinus:
            p1 = ((Binary*)this)->getSubExp1();
            p2 = ((Binary*)this)->getSubExp2();
            p1->partitionTerms(positives, negatives, integers, negate);
            p2->partitionTerms(positives, negatives, integers, !negate);
            break;
        case opTypedExp:
            p1 = ((Binary*)this)->getSubExp1();
            p1->partitionTerms(positives, negatives, integers, negate);
            break;
        case opIntConst: {
            int k = ((Const*)this)->getInt();
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

/*==============================================================================
 * FUNCTION:        [Unary|Binary]::simplifyArith
 * OVERVIEW:        This method simplifies an expression consisting of + and -
 *                  at the top level. For example,
 *                  (%sp + 100) - (%sp + 92) will be simplified to 8.
 * NOTE:            Any expression can be so simplified
 * NOTE:            User must ;//delete result
 * PARAMETERS:      <none>
 * RETURNS:         Ptr to the simplified expression
 *============================================================================*/
Exp* Unary::simplifyArith()
{
    if (op == opMemOf || op == opRegOf || op == opAddrOf || op == opSubscript) {
        // assume we want to simplify the subexpression
        subExp1 = subExp1->simplifyArith();
    }
    return this;            // Else, do nothing
}

Exp* Ternary::simplifyArith() {
    subExp1 = subExp1->simplifyArith();
    subExp2 = subExp2->simplifyArith();
    subExp3 = subExp3->simplifyArith();
    return this;
}

Exp* Binary::simplifyArith() {
    subExp1 = subExp1->simplifyArith();
    subExp2 = subExp2->simplifyArith();
    if ((op != opPlus) && (op != opMinus))
        return this;

    // Partition this expression into positive non-integer terms, negative
    // non-integer terms and integer terms.
    std::list<Exp*> positives;
    std::list<Exp*> negatives;
    std::vector<int> integers;
    partitionTerms(positives,negatives,integers,false);

    // Now reduce these lists by cancelling pairs
    // Note: can't improve this algorithm using multisets, since can't
    // instantiate multisets of type Exp (only Exp*). The Exp* in the
    // multisets would be sorted by address, not by value of the expression.
    // So they would be unsorted, same as lists!
    std::list<Exp*>::iterator pp = positives.begin();
    std::list<Exp*>::iterator nn = negatives.begin();
    while (pp != positives.end()) {
        bool inc = true;
        while (nn != negatives.end()) {
            if (**pp == **nn) {
                // A positive and a negative that are equal; therefore they
                // cancel
                pp = positives.erase(pp);   // Erase the pointers, not the Exps
                nn = negatives.erase(nn);
                inc = false;                // Don't increment pp now
                break;
            }
            nn++;
        }
        if (pp == positives.end()) break;
        if (inc) pp++;
    }

    // Summarise the set of integers to a single number.
    int sum = std::accumulate(integers.begin(),integers.end(),0);

    // Now put all these elements back together and return the result
    if (positives.size() == 0) {
        if (negatives.size() == 0) {
            return new Const(sum);
        } else
            // No positives, some negatives. sum - Acc
            return new Binary(opMinus, new Const(sum),
                Exp::Accumulate(negatives));
    }
    if (negatives.size() == 0) {
        // Positives + sum
        if (sum == 0) {
            // Just positives
            return Exp::Accumulate(positives);
        } else {
            OPER op = opPlus;
            if (sum < 0) {
                op = opMinus;
                sum = -sum;
            }
            return new Binary(op, Exp::Accumulate(positives), new Const(sum));
        }
    }
    // Some positives, some negatives
    if (sum == 0) {
        // positives - negatives
        return new Binary(opMinus, Exp::Accumulate(positives),
            Exp::Accumulate(negatives));
    }
    // General case: some positives, some negatives, a sum
    OPER op = opPlus;
    if (sum < 0) {
        op = opMinus;       // Return (pos - negs) - sum
        sum = -sum;
    }
    return new Binary(op,
        new Binary(opMinus,
            Exp::Accumulate(positives),
            Exp::Accumulate(negatives)),
        new Const(sum));
    
}

/*==============================================================================
 * FUNCTION:        Exp::Accumulate
 * OVERVIEW:        This method creates an expression that is the sum of all
 *                  expressions in a list.
 *                  E.g. given the list <4,r[8],m[14]>
 *                  the resulting expression is 4+r[8]+m[14].
 * NOTE:            static (non instance) function
 * NOTE:            Exps ARE cloned
 * PARAMETERS:      exprs - a list of expressions
 * RETURNS:         a new Exp with the accumulation
 *============================================================================*/
Exp* Exp::Accumulate(std::list<Exp*> exprs)
{
    int n = exprs.size();
    if (n == 0)
        return new Const(0);
    if (n == 1)
        return exprs.front()->clone();
    // 2 or more.
    Binary* res = new Binary(opPlus);
    Binary* cur = res;             // Current expression pointer
    std::list<Exp*>::iterator it;
    int i=1;
    for (it = exprs.begin(); it != exprs.end(); it++, i++) {
        // First term
        cur->setSubExp1((*it)->clone());
        // Second term
        if (i == n-1) {     // 2nd last?
            cur->setSubExp2((*++it)->clone());
        } else {
            cur->setSubExp2(new Binary(opPlus));
            cur = (Binary*)cur->getSubExp2();
        }
    }
    return res;
}

/*==============================================================================
 * FUNCTION:        Exp::simplify
 * OVERVIEW:        Apply various simplifications such as constant folding
 *                  Also canonicalise by putting iteger constants on the right
 *                  hand side of sums, adding of negative constants changed to
 *                  subtracting positive constants, etc.
 *                  Changes << k to a multiply
 * NOTE:            User must ;//delete result
 * NOTE:            Address simplification (a[ m[ x ]] == x) is done separately
 * PARAMETERS:      <none>
 * RETURNS:         Ptr to the simplified expression
 *
 * This code is so big, so weird and so lame it's not funny.  What this boils
 * down to is the process of unification.  We're trying to do it with a simple
 * iterative algorithm, but the algorithm keeps getting more and more complex.
 * Eventually I will replace this with a simple theorem prover and we'll have
 * something powerful, but until then, dont rely on this code to do anything
 * critical. - trent 8/7/2002
 *============================================================================*/
#define DEBUG_SIMP 0            // Set to 1 to print every change
Exp* Exp::simplify() {
#if DEBUG_SIMP
    Exp* save = clone();
#endif
    bool bMod;                  // True if simplified at this or lower level
    Exp* res = this;
    do {
        bMod = false;
        res = res->polySimplify(bMod);// Call the polymorphic simplify
    } while (bMod);             // If modified at this (or a lower) level, redo
    // The below is still important. E.g. want to canonicalise sums, so we
    // know that a + K + b is the same as a + b + K
    // No! This slows everything down, and it's slow enough as it is. Call
    // only where needed:
    // res = res->simplifyArith();
#if DEBUG_SIMP
    if (!(*res == *save)) std::cout << "simplified " << save << "  to  " << res
      << "\n";
    ;//delete save;
#endif
    return res;
}

/*==============================================================================
 * FUNCTION:        Unary::simplify etc
 * OVERVIEW:        Do the work of simplification
 * NOTE:            User must ;//delete result
 * NOTE:            Address simplification (a[ m[ x ]] == x) is done separately
 * PARAMETERS:      <none>
 * RETURNS:         Ptr to the simplified expression
 *============================================================================*/
Exp* Unary::polySimplify(bool& bMod) {
    Exp* res = this;
    subExp1 = subExp1->polySimplify(bMod);

    if (op == opNot || op == opLNot) {
        switch(subExp1->getOper()) {
            case opEquals:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opNotEqual);
                bMod = true;
                return res;
            case opNotEqual:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opEquals);
                bMod = true;
                return res;
            case opLess:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opGtrEq);
                bMod = true;
                return res;
            case opLessEq:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opGtr);
                bMod = true;
                return res;
            case opGtr:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opLessEq);
                bMod = true;
                return res;
            case opGtrEq:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opLess);
                bMod = true;
                return res;
            case opLessUns:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opGtrEqUns);
                bMod = true;
                return res;
            case opLessEqUns:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opGtrUns);
                bMod = true;
                return res;
            case opGtrUns:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opLessEqUns);
                bMod = true;
                return res;
            case opGtrEqUns:
                res = ((Unary*)res)->becomeSubExp1();
                res->setOper(opLessUns);
                bMod = true;
                return res;
            default:
                break;
        }
    }

    switch (op) {
        case opNeg: case opNot: case opLNot: case opSize: {
            OPER subOP = subExp1->getOper();
            if (subOP == opIntConst) {
                // -k, ~k, or !k
                // Note: op is invalid after call to becomeSubExp1() since
                // it ;//deletes this!
                OPER op2 = op;
                res = ((Unary*)res)->becomeSubExp1();
                int k = ((Const*)res)->getInt();
                switch (op2) {
                    case opNeg: k = -k; break; 
                    case opNot: k = ~k; break;
                    case opLNot:k = !k; break;
                    case opSize: /* No change required */ break;
                    default: break;
                }
                ((Const*)res)->setInt(k);
                bMod = true; 
            } else if (op == subOP) {
               res = ((Unary*)res)->becomeSubExp1();
               res = ((Unary*)res)->becomeSubExp1();
               bMod = true;
               break;
            }
        }
        break;
        case opAddrOf:
            // check for a[m[x]], becomes x
            if (subExp1->getOper() == opMemOf) {
                res = ((Unary*)res)->becomeSubExp1();
                res = ((Unary*)res)->becomeSubExp1();
                bMod = true;
                return res;
            }   
            break;
        case opMemOf: case opRegOf: {
            subExp1 = subExp1->polySimplify(bMod);
            // The below IS bad now. It undoes the simplification of
            // m[r29 + -4] to m[r29 - 4]
            // If really needed, do another polySimplify, or swap the order
            //subExp1 = subExp1->simplifyArith();  // probably bad
        }
        break;
        default:
            break;
    }
    return res;
}

Exp* Binary::polySimplify(bool& bMod) {
    Exp* res = this;

    subExp1 = subExp1->polySimplify(bMod);
    subExp2 = subExp2->polySimplify(bMod);

    OPER opSub1 = subExp1->getOper();
    OPER opSub2 = subExp2->getOper();

    if ((opSub1 == opIntConst) && (opSub2 == opIntConst)) {
        // k1 op k2, where k1 and k2 are integer constants
        int k1 = ((Const*)subExp1)->getInt();
        int k2 = ((Const*)subExp2)->getInt();
        bool change = true;
        switch (op) {
            case opPlus:    k1 = k1 + k2; break;
            case opMinus:   k1 = k1 - k2; break;
            case opDiv:     k1 = (int) ((unsigned)k1 / (unsigned)k2);
            case opDivs:    k1 = k1 / k2; break;
            case opMod:     k1 = (int) ((unsigned)k1 % (unsigned)k2);
            case opMods:    k1 = k1 % k2; break;
            case opMult:    k1 = (int) ((unsigned)k1 * (unsigned)k2); break;
            case opMults:   k1 = k1 * k2; break;
            case opShiftL:  k1 = k1 << k2; break;
            case opShiftR:  k1 = k1 >> k2; break;
            case opShiftRA: k1 = (k1 >> k2) |
                                (((1 << k2) -1) << (32 - k2));
                                break;
            case opBitOr:       k1 = k1 | k2; break;
            case opBitAnd:      k1 = k1 & k2; break;
            case opBitXor:      k1 = k1 ^ k2; break;
            case opEquals:      k1 = (k1 == k2); break;
            case opNotEqual:    k1 = (k1 != k2); break;
            case opLess:        k1 = (k1 <  k2); break;
            case opGtr:         k1 = (k1 >  k2); break;
            case opLessEq:      k1 = (k1 <= k2); break;
            case opGtrEq:       k1 = (k1 >= k2); break;
            case opLessUns:     k1 = ((unsigned)k1 < (unsigned)k2); break;
            case opGtrUns:      k1 = ((unsigned)k1 > (unsigned)k2); break;
            case opLessEqUns:   k1 = ((unsigned)k1 <=(unsigned)k2); break;
            case opGtrEqUns:    k1 = ((unsigned)k1 >=(unsigned)k2); break;
            default: change = false;
        }
        if (change) {
            ;//delete res;
            res = new Const(k1);
            bMod = true;
            return res;
        }
    }

    if (((op == opBitXor) || (op == opMinus)) && (*subExp1 == *subExp2)) {
        // x ^ x or x - x: result is zero
        ;//delete this;
        res = new Const(0);
        bMod = true;
        return res;
    }
        
    if (op == opEquals && *subExp1 == *subExp2) {
        // x == x: result is true
        ;//delete this;
        res = new Terminal(opTrue);
        bMod = true;
        return res;
    }

    // Might want to commute to put an integer constant on the RHS
    // Later simplifications can rely on this (ADD other ops as necessary)
    if (opSub1 == opIntConst && 
          (op == opPlus || op == opMult   || op == opMults || op == opBitOr ||
           op == opBitAnd )) {
        commute();
        // Swap opSub1 and opSub2 as well
        OPER t = opSub1;
        opSub1 = opSub2;
        opSub2 = t;
        // This is not counted as a modification
    }

    // Similarly for boolean constants
    if (subExp1->isBoolConst() && !subExp2->isBoolConst() &&
          (op == opAnd || op == opOr)) {
        commute();
        // Swap opSub1 and opSub2 as well
        OPER t = opSub1;
        opSub1 = opSub2;
        opSub2 = t;
        // This is not counted as a modification
    }

    // check for (x + a) + b where a and b are constants, becomes x + a+b
    if (op == opPlus && opSub1 == opPlus && opSub2 == opIntConst &&
        subExp1->getSubExp2()->getOper() == opIntConst) {
        int n = ((Const*)subExp2)->getInt();
        res = ((Binary*)res)->becomeSubExp1();
        ((Const*)res->getSubExp2())->setInt(
            ((Const*)res->getSubExp2())->getInt() + n);
        bMod = true;
        return res;
    }

    // check for (x - a) + b where a and b are constants, becomes x + -a+b
    if (op == opPlus && opSub1 == opMinus && opSub2 == opIntConst &&
        subExp1->getSubExp2()->getOper() == opIntConst) {
        int n = ((Const*)subExp2)->getInt();
        res = ((Binary*)res)->becomeSubExp1();
        res->setOper(opPlus);
        ((Const*)res->getSubExp2())->setInt(
            (-((Const*)res->getSubExp2())->getInt()) + n);
        bMod = true;
        return res;
    }


    // Turn a + -K into a - K (K is int const > 0)
    // Also a - -K into a + K (K is int const > 0)
    // Does not count as a change
    if ((op == opPlus || op == opMinus) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() < 0) {
        ((Const*)subExp2)->setInt(-((Const*)subExp2)->getInt());
        op = op == opPlus ? opMinus : opPlus;
    }

    // Check for exp + 0  or  exp - 0  or  exp | 0
    if ((op == opPlus || op == opMinus || op == opBitOr) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0) {
        res = ((Binary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp or false
    if (op == opOr && subExp2->isFalse()) {
        res = ((Binary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }
       
    // Check for exp * 0  or exp & 0
    if ((op == opMult || op == opMults || op == opBitAnd) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0) {
        ;//delete res;
        res = new Const(0);
        bMod = true;
        return res;
    }

    // Check for exp and false
    if (op == opAnd && subExp2->isFalse()) {
        ;//delete res;
        res = new Terminal(opFalse);
        bMod = true;
        return res;
    }

    // Check for exp * 1
    if ((op == opMult || op == opMults) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 1) {
        res = ((Unary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp AND -1 (bitwise AND)
    if ((op == opBitAnd) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() == -1) {
        res = ((Unary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp AND TRUE (logical AND)
    if ((op == opAnd) &&
      // Is the below really needed?
      (opSub2 == opIntConst && ((Const*)subExp2)->getInt() != 0) ||
       subExp2->isTrue()) {
        res = ((Unary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    // Check for exp OR TRUE (logical OR)
    if ((op == opOr) &&
      (opSub2 == opIntConst && ((Const*)subExp2)->getInt() != 0) ||
       subExp2->isTrue()) {
        ;//delete res;
        res = new Terminal(opTrue);
        bMod = true;
        return res;
    }

    // Check for [exp] << k where k is a positive integer const
    int k;
    if (op == opShiftL && opSub2 == opIntConst &&
      ((k = ((Const*)subExp2)->getInt(), (k >= 0 && k < 32)))) {
        res->setOper(opMult);
        ((Const*)subExp2)->setInt(1 << k);
        bMod = true;
        return res;
    }

/*
    // Check for -x compare y, becomes x compare -y
    // doesn't count as a change
    if (isComparison() && opSub1 == opNeg) {
        Exp *e = subExp1;
        subExp1 = e->getSubExp1()->clone();
        ;//delete e;
        subExp2 = new Unary(opNeg, subExp2);
    }

    // Check for (x + y) compare 0, becomes x compare -y
    if (isComparison() &&
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
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 1 && opSub1 == opEquals) {
        ;//delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        ;//delete b;
        bMod = true;
        return res;
    }

    // Check for x + -y == 0, becomes x == y
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 0 && opSub1 == opPlus &&
        ((Binary*)subExp1)->subExp2->getOper() == opIntConst) {
        Binary *b = (Binary*)subExp1;
        int n = ((Const*)b->subExp2)->getInt();
        if (n < 0) {
            ;//delete subExp2;
            subExp2 = b->subExp2;
            ((Const*)subExp2)->setInt(-((Const*)subExp2)->getInt());
            b->subExp2 = 0;
            subExp1 = b->subExp1;
            b->subExp1 = 0;
            ;//delete b;
            bMod = true;
            return res;
        }
    }

    // Check for (x == y) == 0, becomes x != y
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 0 && opSub1 == opEquals) {
        ;//delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        ;//delete b;
        bMod = true;
        res->setOper(opNotEqual);
        return res;
    }

    // Check for (x == y) != 1, becomes x != y
    if (op == opNotEqual && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 1 && opSub1 == opEquals) {
        ;//delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        ;//delete b;
        bMod = true;
        res->setOper(opNotEqual);
        return res;
    }

    // Check for (x == y) != 0, becomes x == y
    if (op == opNotEqual && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 0 && opSub1 == opEquals) {
        res = ((Binary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }


    // Check for (x > y) == 0, becomes x <= y
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 0 && opSub1 == opGtr) {
        ;//delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        ;//delete b;
        bMod = true;
        res->setOper(opLessEq);
        return res;
    }

    // Check for (x >u y) == 0, becomes x <=u y
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 0 && opSub1 == opGtrUns) {
        ;//delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        ;//delete b;
        bMod = true;
        res->setOper(opLessEqUns);
        return res;
    }

    Binary *b1 = (Binary*)subExp1;
    Binary *b2 = (Binary*)subExp2;
    // Check for (x <= y) || (x == y), becomes x <= y
    if (op == opOr && opSub2 == opEquals &&
        (opSub1 == opGtrEq || opSub1 == opLessEq ||
         opSub1 == opGtrEqUns || opSub1 == opLessEqUns) &&
        ((*b1->subExp1 == *b2->subExp1 &&
          *b1->subExp2 == *b2->subExp2) || 
         (*b1->subExp1 == *b2->subExp2 &&
          *b1->subExp2 == *b2->subExp1))) {
        res = ((Binary*)res)->becomeSubExp1();
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
        res = ((Binary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    // check for a + a*n, becomes a*(n+1) where n is an int
    if (op == opPlus && opSub2 == opMult && *subExp1 == *subExp2->getSubExp1() &&
        subExp2->getSubExp2()->getOper() == opIntConst) {
        res = ((Binary*)res)->becomeSubExp2();
        ((Const*)res->getSubExp2())->setInt(((Const*)res->getSubExp2())->getInt()+1);
        bMod = true;
        return res;     
    }

    // check for a*n*m, becomes a*(n*m) where n and m are ints
    if (op == opMult && opSub1 == opMult && opSub2 == opIntConst && 
        subExp1->getSubExp2()->getOper() == opIntConst) {
        int m = ((Const*)subExp2)->getInt();
        res = ((Binary*)res)->becomeSubExp1();
        ((Const*)res->getSubExp2())->setInt(((Const*)res->getSubExp2())->getInt()*m);
        bMod = true;
        return res;
    }

    // check for !(a == b) becomes a != b
    if (op == opLNot && opSub1 == opEquals) {
        res = ((Unary*)res)->becomeSubExp1();
        res->setOper(opNotEqual);
        bMod = true;
        return res;
    }

    // check for !(a != b) becomes a == b
    if (op == opLNot && opSub1 == opNotEqual) {
        res = ((Unary*)res)->becomeSubExp1();
        res->setOper(opEquals);
        bMod = true;
        return res;
    }

    return res;
}

Exp* Ternary::polySimplify(bool& bMod) {
    Exp *res = this;

    subExp1 = subExp1->polySimplify(bMod);
    subExp2 = subExp2->polySimplify(bMod);
    subExp3 = subExp3->polySimplify(bMod);

    if (op == opTern && subExp2->getOper() == opIntConst && 
        subExp3->getOper() == opIntConst) {
        Const *s2 = (Const*)subExp2;
        Const *s3 = (Const*)subExp3;

        if (s2->getInt() == 1 && s3->getInt() == 0) {
            res = this->becomeSubExp1();
            bMod = true;
            return res;
        }
    }   

    if (op == opSgnEx && subExp3->getOper() == opIntConst) {
        res = this->becomeSubExp3();
        bMod = true;
        return res;
    }

    if (op == opFsize && subExp3->getOper() == opItof &&
        *subExp1 == *subExp3->getSubExp2() &&
        *subExp2 == *subExp3->getSubExp1()) {
        res = this->becomeSubExp3();
        bMod = true;
        return res;
    }

    return res;
}

Exp* TypedExp::polySimplify(bool& bMod) {
    Exp *res = this;
    Exp *subExp1 = getSubExp1();
    
    if (subExp1->getOper() == opRegOf) {
        // type cast on a reg of.. hmm.. let's remove this
        res = ((Unary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    return res;
}

Exp* PhiExp::polySimplify(bool& bMod) {
    Exp *res = this;
    Exp *subExp1 = getSubExp1()->polySimplify(bMod);

    if (stmtVec.begin() != stmtVec.end()) {
        bool allSame = true;
        StatementVec::iterator uu;
        for (uu = stmtVec.begin(); uu != stmtVec.end(); uu++) {
            if (*uu != *stmtVec.begin()) {
                allSame = false;
                break;
            }
        }

        if (allSame) {
            if (VERBOSE)
                LOG << "all the same in " << this << "\n";
            bMod = true;
            res = new RefExp(subExp1, *stmtVec.begin());
            return res;
        }

        bool onlyOneNotThis = true;
        Statement *silly = new Assign();
        Statement *notthis = silly;
        for (uu = stmtVec.begin(); uu != stmtVec.end(); uu++) {
            if (*uu == NULL || !(*uu)->isPhi() || (*uu)->getRight() != this)
                if (notthis != silly) {
                    onlyOneNotThis = false;
                    break;
                } else notthis = *uu;
        }

        if (onlyOneNotThis && notthis != silly) {
            if (VERBOSE)
                LOG << "all but one not this in " << this << "\n";
            bMod = true;
            res = new RefExp(subExp1, notthis);
            return res;
        }
    }

    return res;
}

/*==============================================================================
 * FUNCTION:        Exp::simplifyAddr
 * OVERVIEW:        Just do addressof simplification: a[ m[ any ]] == any,
 *                    and also a[ size m[ any ]] == any
 * PARAMETERS:      <none>
 * RETURNS:         Ptr to the simplified expression
 *============================================================================*/
Exp* Unary::simplifyAddr() {
    if (op != opAddrOf) {
        // Not a[ anything ]. Recurse
        subExp1 = subExp1->simplifyAddr();
        return this;
    }
    if (subExp1->getOper() == opMemOf) {
        Unary* s = (Unary*)becomeSubExp1();
        return s->becomeSubExp1();
    }
    if (subExp1->getOper() == opSize) {
        Exp* sub = subExp1->getSubExp2();
        if (sub->getOper() == opMemOf) {
            // Remove the a[
            Binary* b = (Binary*)becomeSubExp1();
            // Remove the size[
            Unary* u = (Unary*)b->becomeSubExp2();
            // Remove the m[
            return u->becomeSubExp1();
        }
    }

    // a[ something else ]. Still recurse, just in case
    subExp1 = subExp1->simplifyAddr();
    return this;
} 

Exp* Binary::simplifyAddr() {
    subExp1 = subExp1->simplifyAddr();
    subExp2 = subExp2->simplifyAddr();
    return this;
}

Exp* Ternary::simplifyAddr() {
    subExp1 = subExp1->simplifyAddr();
    subExp2 = subExp2->simplifyAddr();
    subExp3 = subExp3->simplifyAddr();
    return this;
}

/*==============================================================================
 * FUNCTION:        Exp::printt
 * OVERVIEW:        Print an infix representation of the object to the given
 *                  file stream, with its type in <angle brackets>.
 * PARAMETERS:      Output stream to send the output to
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::printt(std::ostream& os /*= cout*/, bool withUses /* = false */)
{
    print(os, withUses);
    if (op != opTypedExp) return;
    Type* t = ((TypedExp*)this)->getType();
    os << "<" << std::dec << t->getSize();
/*    switch (t->getType()) {
        case INTEGER:
            if (t->getSigned())
                        os << "i";              // Integer
            else
                        os << "u"; break;       // Unsigned
        case FLOATP:    os << "f"; break;
        case DATA_ADDRESS: os << "pd"; break;   // Pointer to Data
        case FUNC_ADDRESS: os << "pc"; break;   // Pointer to Code
        case VARARGS:   os << "v"; break;
        case TBOOLEAN:   os << "b"; break;
        case UNKNOWN:   os << "?"; break;
        case TVOID:     break;
    } */
    os << ">";
}

/*==============================================================================
 * FUNCTION:        Exp::printAsHL
 * OVERVIEW:        Print an infix representation of the object to the given
 *                  file stream, but convert r[10] to r10 and v[5] to v5
 * NOTE:            Never modify this function to emit debugging info; the back
 *                    ends rely on this being clean to emit correct C
 *                    If debugging is desired, use operator<<
 * PARAMETERS:      Output stream to send the output to
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::printAsHL(std::ostream& os /*= cout*/)
{
    std::ostringstream ost;
    ost << this;                    // Print to the string stream
    std::string s(ost.str());
    if ((s.length() >= 4) && (s[1] == '[')) {
        // r[nn]; change to rnn
        s.erase(1, 1);              // '['
        s.erase(s.length()-1);      // ']'
    }
    os << s;                        // Print to the output stream
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for Exp*
 * PARAMETERS:      os: output stream to send to
 *                  p: ptr to Exp to print to the stream
 * RETURNS:         copy of os (for concatenation)
 *============================================================================*/
std::ostream& operator<<(std::ostream& os, Exp* p)
{
#if 1
    // Useful for debugging, but can clutter the output
    p->printt(os, true);
#else
    p->print(os, true);
#endif
    return os;
}

// Replace all e in this Exp with e{def}
Exp* Unary::expSubscriptVar(Exp* e, Statement* def) {
    subExp1 = subExp1->expSubscriptVar(e, def);
    if (*e == *this || e->getOper() == opWild)
        return new RefExp(this, def);
    return this;
}
Exp* Terminal::expSubscriptVar(Exp* e, Statement* def) {
    if (*e == *this || e->getOper() == opWild)
        return new RefExp(this, def);
    return this;
}
Exp* Binary::expSubscriptVar(Exp* e, Statement* def) {
    subExp1 = subExp1->expSubscriptVar(e, def);
    subExp2 = subExp2->expSubscriptVar(e, def);
    return this;
}
Exp* Ternary::expSubscriptVar(Exp* e, Statement* def) {
    subExp1 = subExp1->expSubscriptVar(e, def);
    subExp2 = subExp2->expSubscriptVar(e, def);
    subExp3 = subExp3->expSubscriptVar(e, def);
    return this;
}
Exp* RefExp::expSubscriptVar(Exp* e, Statement* def) {
    if (*e == *subExp1 || e->getOper() == opWild)
        this->def = def;
    else
        // Need the else, otherwise in changing r28{3} to r28{4} will
        // change again to r28{4}{4}
        subExp1 = subExp1->expSubscriptVar(e, def);
    return this;
}

/*==============================================================================
 * FUNCTION:        Unary::fixSuccessor
 * OVERVIEW:        Replace succ(r[k]) by r[k+1]
 * NOTE:            Could change top level expression
 * PARAMETERS:      None
 * RETURNS:         Fixed expression
 *============================================================================*/
static Unary succRegOf(opSuccessor,
    new Unary(opRegOf, new Terminal(opWild)));
Exp* Exp::fixSuccessor() {
    bool change;
    Exp* result;
    // Assume only one successor function in any 1 expression
    if (search(&succRegOf, result)) {
        // Result has the matching expression, i.e. succ(r[K])
        Exp* sub1 = ((Unary*)result)->getSubExp1();
        assert(sub1->getOper() == opRegOf);
        Exp* sub2 = ((Unary*)sub1)->getSubExp1();
        assert(sub2->getOper() == opIntConst);
        // result    sub1   sub2
        // succ(      r[   Const K  ])
        // Note: we need to clone the r[K] part, since it will be ;//deleted as
        // part of the searchReplace below
        Unary* replace = (Unary*)sub1->clone();
        Const* c = (Const*)replace->getSubExp1();
        c->setInt(c->getInt()+1);       // Do the increment
        Exp* res = searchReplace(result, replace, change);
        return res;
    }
    return this;
}
    
/*==============================================================================
 * FUNCTION:        Exp::killFill
 * OVERVIEW:        Remove size operations such as zero fill, sign extend
 * NOTE:            Could change top level expression
 * NOTE:            Does not handle truncation at present
 * PARAMETERS:      None
 * RETURNS:         Fixed expression
 *============================================================================*/
static Ternary srch1(opZfill, new Terminal(opWild), new Terminal(opWild),
    new Terminal(opWild));
static Ternary srch2(opSgnEx, new Terminal(opWild), new Terminal(opWild),
    new Terminal(opWild));
Exp* Exp::killFill() {
    Exp* res = this;
    std::list<Exp**> result;
    doSearch(&srch1, res, result, false);
    doSearch(&srch2, res, result, false);
    std::list<Exp**>::iterator it;
    for (it = result.begin(); it != result.end(); it++) {
        // Kill the sign extend bits
        **it = ((Ternary*)(**it))->becomeSubExp3();
    }
    return res;
}

bool Exp::isTemp() {
    if (op == opTemp) return true;
    if (op != opRegOf) return false;
    // Some old code has r[tmpb] instead of just tmpb
    Exp* sub = ((Unary*)this)->getSubExp1();
    return sub->op == opTemp;
}

void Unary::addUsedLocs(LocationSet& used) {
    switch (op) {
        // We are interested in r[], m[], and variables (named arguments,
        // locals, and globals)
        case opRegOf:   case opMemOf:
        case opArg:     case opLocal:   case opGlobal:  case opParam:
            // We want to add this expression
            used.insert(clone());
            // We also need to recurse, in case we have m[m[...]] or m[r[...]]
            if (op == opMemOf)
                subExp1->addUsedLocs(used);
            break;
        default:
            break;
    }
    subExp1->addUsedLocs(used);
}

void Terminal::addUsedLocs(LocationSet& used) {
    switch (op) {
        case opPC:
        case opFlags:
        // Fall through
        // The carry flag can be used in some SPARC idioms, etc
        case opCF:
            used.insert(clone());
        default:
            break;
    }
}


void Binary::addUsedLocs(LocationSet& used) {
    subExp1->addUsedLocs(used);
    subExp2->addUsedLocs(used);
}

void Ternary::addUsedLocs(LocationSet& used) {
    subExp1->addUsedLocs(used);
    subExp2->addUsedLocs(used);
    subExp3->addUsedLocs(used);
}

void RefExp::addUsedLocs(LocationSet& used) {
    used.insert(clone());           // We want to see these
    if (subExp1->isMemOf()) {
        Exp* grandChild = ((Unary*)subExp1)->getSubExp1();
        grandChild->addUsedLocs(used);
    }
}

void PhiExp::addUsedLocs(LocationSet& used) {
    /* Suppose we have
      10: r1 := 5
      20: {r1,r2} := call(foo)
      30: r1 := phi{10 20}
      That means we effectively use r1{10} and r1{20}
    */
    StatementVec::iterator uu;
    for (uu = stmtVec.begin(); uu != stmtVec.end(); uu++) {
        Exp* temp = new RefExp(subExp1->clone(), *uu);
        used.insert(temp);
    }
}

Exp *Exp::removeSubscripts(bool& allZero)
{
    Exp *e = this;
    LocationSet locs;
    e->addUsedLocs(locs);
    LocationSet::iterator xx; 
    allZero = true;
    for (xx = locs.begin(); xx != locs.end(); xx++) {
        if ((*xx)->getOper() == opSubscript) {
            RefExp *r1 = (RefExp*)*xx;
            if (r1->getRef() != NULL) {
                allZero = false;
            }
            bool change; 
            e = e->searchReplaceAll(*xx, r1->getSubExp1()->clone(), change);
        }
    }
    return e;
}


Exp *Unary::fixCallRefs()
{
    subExp1 = subExp1->fixCallRefs();
    return this->simplify();
}

Exp *Binary::fixCallRefs()
{
    subExp1 = subExp1->fixCallRefs();
    subExp2 = subExp2->fixCallRefs();
    return this->simplify();
}

Exp *Ternary::fixCallRefs()
{
    subExp1 = subExp1->fixCallRefs();
    subExp2 = subExp2->fixCallRefs();
    subExp3 = subExp3->fixCallRefs();
    return this->simplify();
}

Exp *RefExp::fixCallRefs() {
    subExp1 = subExp1->fixCallRefs();
    CallStatement *call = dynamic_cast<CallStatement*>(def);
    if (call) {
        Exp *e = call->getProven(subExp1);
        if (e) {
            e = call->substituteParams(e->clone());
            assert(e);
            if (VERBOSE)
                LOG << "fixcall refs replacing " << this << " with " << e 
                    << "\n";
            ;//delete this;
            return e->simplify();
        } else {
            if (call->findReturn(subExp1) == -1) {
                if (VERBOSE) {
                    LOG << "nothing proven about " << subExp1 << 
                        " and yet it is referenced, and not in returns of " << 
                        "\n" << "   " << call << "\n";
                }
            }
        }
    }
    return this->simplify();
}

// This is a hack.  If we have a phi which has one of its
// elements referencing a statement which is defined as a 
// function address, then we can use this information to
// resolve references to indirect calls more aggressively.
// Note that this is not technically correct and will give
// the wrong result if the callee of an indirect call
// actually modifies a function pointer in the caller. 
bool PhiExp::hasGlobalFuncParam(Prog *prog)
{
    unsigned n = stmtVec.size();
    for (unsigned i = 0; i < n; i++) {
        Statement* u = stmtVec.getAt(i);
        if (u == NULL) continue;
        Exp *right = u->getRight();
        if (right == NULL)
            continue;
        if (right->getOper() == opGlobal ||
            (right->getOper() == opSubscript &&
             right->getSubExp1()->getOper() == opGlobal)) {
            Exp *e = right;
            if (right->getOper() == opSubscript)
                e = right->getSubExp1();
            char *nam = ((Const*)e->getSubExp1())->getStr();
            Proc *p = prog->findProc(nam);
            if (p == NULL)
                p = prog->getLibraryProc(nam);
            if (p) {
                if (VERBOSE)
                    LOG << "statement " << i << " of " << this 
                        << " is a global func\n";
                return true;
            }
        }
        // BAD: this can loop forever if we have a phi loop
        if (u->isPhi() && u->getRight() != this &&
            ((PhiExp*)u->getRight())->hasGlobalFuncParam(prog))
            return true;
    }
    return false;
}

Exp *PhiExp::fixCallRefs() {
    std::vector<Statement*> remove;
    std::vector<Statement*> insert;
    unsigned n = stmtVec.size();

    bool oneIsGlobalFunc = false;
    Prog *prog = NULL;
    for (unsigned i=0; i < n; i++) {
        Statement* u = stmtVec.getAt(i);
        CallStatement *call = dynamic_cast<CallStatement*>(u);
        if (call)
            prog = call->getProc()->getProg();
    }
    if (prog) 
        oneIsGlobalFunc = hasGlobalFuncParam(prog);

    for (unsigned i=0; i < n; i++) {
        Statement* u = stmtVec.getAt(i);
        CallStatement *call = dynamic_cast<CallStatement*>(u);
        if (call) {
            Exp *e = call->getProven(subExp1);
            if (call->isComputed() && oneIsGlobalFunc) {
                e = subExp1->clone();
                if (VERBOSE)
                    LOG << "ignoring ref in phi to computed call with "
                        << "function pointer param " << e << "\n";
            }
            if (e) {
                e = call->substituteParams(e->clone());
                if (e && e->getOper() == opSubscript &&
                    *e->getSubExp1() == *subExp1) {
                    if (VERBOSE)
                        LOG << "fixcall refs replacing param " << i << " in "
                            << this << " with " << e << "\n";
                    stmtVec.putAt(i, ((RefExp*)e)->getRef());
                } else {
                    if (VERBOSE)
                        LOG << "cant update phi ref to " << e 
                                  << "\n";
                }
            } else {
                if (call->findReturn(subExp1) == -1) {
                    if (VERBOSE) {
                        LOG << "nothing proven about " << subExp1 << 
                            " and yet it is referenced, and not in returns of " 
                            << "\n" << "   " << call << "\n";
                    }
                }
            }
        }
    }
    return this->simplify();
}

//
// From SSA form
//

Exp* RefExp::fromSSA(igraph& ig) {
    // FIXME: Need to check if the argument is a memof, and if so
    // deal with that specially (e.g. global)
    // Check to see if it is in the map
    igraph::iterator it = ig.find(this);
    if (it == ig.end()) {
        // It is not in the map. Remove the opSubscript
        Exp* ret = becomeSubExp1();
        // ret could be a memof, etc, which could need subscripts removed from
        ret = ret->fromSSA(ig);
        return ret;
    }
    else {
        // It is in the map. Delete the current expression, and replace
        // with a new local
        std::ostringstream os;
        os << "local" << ig[this];
        std::string name = os.str();
        ;//delete this;
        return new Unary(opLocal, new Const(strdup(name.c_str())));
    }
}

Exp* PhiExp::fromSSA(igraph& ig) {
    // Almost nothing to be done yet; see UserProc::fromSSAform()
    // Don't rename the top level, but if it's a unary (esp m[]),
    // transform the grandchild
    if (subExp1->getArity() == 1) {
        Exp* grandChild = ((Unary*)subExp1)->getSubExp1();
        Exp* newG = grandChild->fromSSA(ig);
        if (newG != grandChild)
            ((Unary*)subExp1)->setSubExp1ND(newG);
    }
    return this;
}


Exp* Unary::fromSSA(igraph& ig) {
    subExp1 = subExp1->fromSSA(ig);
    return this;
}

Exp* Binary::fromSSA(igraph& ig) {
    subExp1 = subExp1->fromSSA(ig);
    subExp2 = subExp2->fromSSA(ig);
    return this;
}

Exp* Ternary::fromSSA(igraph& ig) {
    subExp1 = subExp1->fromSSA(ig);
    subExp2 = subExp2->fromSSA(ig);
    subExp3 = subExp3->fromSSA(ig);
    return this;
}

Exp* Exp::fromSSAleft(igraph& ig, Statement* d) {
    RefExp* r = new RefExp(this, d);       // "Wrap" in a ref
    return r->fromSSA(ig);
    // Note: r will be ;//deleted in fromSSA! Do not ;//delete here!
}

// Return the memory nesting depth
// Examples: r[24] returns 0; m[r[24 + m[r[25]] + 4] returns 2
int Unary::getMemDepth() {
    if (op == opMemOf)
        return subExp1->getMemDepth() + 1;
    else
        return subExp1->getMemDepth();
}

int Binary::getMemDepth() {
    int d1 = subExp1->getMemDepth();
    int d2 = subExp2->getMemDepth();
    return std::max(d1, d2);
}

int Ternary::getMemDepth() {
    int d1 = subExp1->getMemDepth();
    int d2 = subExp2->getMemDepth();
    int d3 = subExp2->getMemDepth();
    return std::max(std::max(d1, d2), d3);
}

    

// A helper file for comparing Exp*'s sensibly
bool lessExpStar::operator()(const Exp* x, const Exp* y) const {
    return (*x < *y);       // Compare the actual Exps
}

bool lessTI::operator()(const Exp* x, const Exp* y) const {
    return (*x << *y);      // Compare the actual Exps
}

//  //  //  //  //  //
//  genConstraints  //
//  //  //  //  //  //

Exp* Exp::genConstraints(Exp* result) {
    // Default case, no constraints -> return true
    return new Terminal(opTrue);
}

Exp* Const::genConstraints(Exp* result) {
    if (result->isTypeVal()) {
        // result is a constant type, or possibly a partial type such as
        // ptr(alpha)
        Type* t = ((TypeVal*)result)->getType();
        bool match = false;
        switch (op) {
            case opIntConst:
            case opLongConst:
                // What about sizes?
                match = t->isInteger();
                // An integer constant can also match a pointer to something
                match |= t->isPointer();
                break;
            case opStrConst:
                match = (t->isPointer()) &&
                  ((PointerType*)t)->getPointsTo()->isChar();
                break;
            case opFltConst:
                match = t->isFloat();
                break;
            default:
                break;
        }
        if (match)
            return new Terminal(opTrue);
        else {
            // This constant will require a cast. So we generate a constraint
            // as a sign to the back end
            return new Binary(opEquals,
                new Unary(opTypeOf, this->clone()),
                result->clone());
        }
    }
    // result is a type variable, which is constrained by this constant
    Type* t;
    switch (op) {
        case opIntConst:
            // Can be integer, but a pointer to anything
            return new Binary(opOr,
                new Binary(opEquals,
                    result,
                    new TypeVal(new IntegerType(32))),
                new Binary(opEquals,
                    result,
                    new TypeVal(PointerType::newPtrAlpha())));
            break;
        case opLongConst:
            t = new IntegerType(64);
            break;
        case opStrConst:
            t = new PointerType(new CharType());
            break;
        case opFltConst:
            t = new FloatType();    // size?
            break;
        default:
            return false;
    }
    TypeVal* tv = new TypeVal(t);
    Exp* e = new Binary(opEquals, result->clone(), tv);
    return e;
}

Exp* Unary::genConstraints(Exp* result) {
    switch (op) {
        case opRegOf:
        case opParam:
        case opGlobal:
        case opLocal:
            return new Binary(opEquals,
                new Unary(opTypeOf, this->clone()),
                result->clone());
        default:
            break;
    }
    return new Terminal(opTrue);
}

Exp* RefExp::genConstraints(Exp* result) {
    OPER subOp = subExp1->getOper();
    switch (subOp) {
        case opRegOf:
        case opParam:
        case opGlobal:
        case opLocal:
            return new Binary(opEquals,
                new Unary(opTypeOf, this->clone()),
                result->clone());
        default:
            break;
    }
    return new Terminal(opTrue);
}

Exp* Binary::constrainSub(TypeVal* typeVal1, TypeVal* typeVal2) {
    Exp* con1 = subExp1->genConstraints(typeVal1);
    Exp* con2 = subExp2->genConstraints(typeVal2);
    return new Binary(opAnd, con1, con2);
}

Exp* Binary::genConstraints(Exp* result) {
    Type* restrictTo = NULL;
    if (result->isTypeVal())
        restrictTo = ((TypeVal*)result)->getType();
    Exp* res = NULL;
    IntegerType* intType = new IntegerType;
    TypeVal intVal(intType);
    switch (op) {
        case opFPlus:
        case opFMinus:
        case opFMult:
        case opFDiv: {
            if (restrictTo && !restrictTo->isFloat())
                // Result can only be float
                return new Terminal(opFalse);

            FloatType* ft = new FloatType();
            TypeVal* ftv = new TypeVal(ft);
            res = constrainSub(ftv, ftv);
            if (!restrictTo)
                // Also constrain the result
                res = new Binary(opAnd, res,
                    new Binary(opEquals, result->clone(), ftv));
            else
                ;//delete ftv;     // Also ;//deletes ft
            return res;
            break;
        }

        case opPlus: {
            // A pointer to anything
            Type* ptrType = PointerType::newPtrAlpha();
            TypeVal ptrVal(ptrType);    // Type value of ptr to anything
            if (!restrictTo || restrictTo && restrictTo->isInteger()) {
                // int + int -> int
                res = constrainSub(&intVal, &intVal);
                if (!restrictTo)
                    res = new Binary(opAnd, res,
                        new Binary(opEquals, result->clone(),
                        intVal.clone()));
            }

            if (!restrictTo || restrictTo && restrictTo->isPointer()) {
                // ptr + int -> ptr
                Exp* res2 = constrainSub(&ptrVal, &intVal);
                if (!restrictTo)
                    res2 = new Binary(opAnd, res2,
                        new Binary(opEquals, result->clone(),
                        ptrVal.clone()));
                if (res) res = new Binary(opOr, res, res2);
                else     res = res2;

                // int + ptr -> ptr
                res2 = constrainSub(&intVal, &ptrVal);
                if (!restrictTo)
                    res2 = new Binary(opAnd, res2,
                        new Binary(opEquals, result->clone(),
                        ptrVal.clone()));
                if (res) res = new Binary(opOr, res, res2);
                else     res = res2;
            }

            if (res) return res->simplify();
            else return new Terminal(opFalse);
        }
            
        case opMinus: {
            Type* ptrType = 0;      // FIXME!
            TypeVal ptrVal(ptrType);
            if (!restrictTo || restrictTo && restrictTo->isInteger()) {
                // int - int -> int
                res = constrainSub(&intVal, &intVal);
                if (!restrictTo)
                    res = new Binary(opAnd, res,
                        new Binary(opEquals, result->clone(),
                        intVal.clone()));

                // ptr - ptr -> int
                Exp* res2 = constrainSub(&ptrVal, &ptrVal);
                if (!restrictTo)
                    res2 = new Binary(opAnd, res2,
                        new Binary(opEquals, result->clone(),
                        ptrVal.clone()));
                if (res) res = new Binary(opOr, res, res2);
                else     res = res2;
            }

            if (!restrictTo || restrictTo && restrictTo->isPointer()) {
                // ptr - int -> ptr
                Exp* res2 = constrainSub(&ptrVal, &intVal);
                if (!restrictTo)
                    res2 = new Binary(opAnd, res2,
                        new Binary(opEquals, result->clone(),
                        ptrVal.clone()));
                if (res) res = new Binary(opOr, res, res2);
                else     res = res2;
            }

            if (res) return res->simplify();
            else return new Terminal(opFalse);
        }
            
                
        default:
            break;
    }
    return new Terminal(opTrue);
}

Exp* PhiExp::genConstraints(Exp* result) {
    // Generate a constraint that all the phi's have to be the same type as
    // result
    assert(result->isTypeOf());
    Exp* base = ((Unary*)result)->getSubExp1();
    assert(base->isSubscript());
    base = ((RefExp*)base)->getSubExp1();
    StatementVec::iterator uu;
    Exp* ret = new Terminal(opTrue);
    bool first = true;
    for (uu = stmtVec.begin(); uu != stmtVec.end(); uu++) {
        Exp* conjunct = new Binary(opEquals,
            result,
            new Unary(opTypeOf,
                new RefExp(base, *uu)));
        if (first) {
            ret = conjunct;
            first = false;
        }
        else
            ret = new Binary(opAnd, ret, conjunct);
    }
    return ret->simplify();
}

Exp* Ternary::genConstraints(Exp* result) {
    return new Terminal(opTrue);
}
