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
 *              variable was used after "this" had been deleted
 * 10 Jul 02 - Mike: Added simplifyAddr() methods
 * 16 Jul 02 - Mike: Fixed memory issues with operator==
 * ?? Nov 02 - Mike: Added Exp::prints (great for debugging)
 * 26 Nov 02 - Mike: Quelched some warnings; fixed an error in AssignExp copy
 *              constructor
 * 03 Dec 02 - Mike: Fixed simplification of exp AND -1 (was exp AND +1)
 * 09 Dec 02 - Mike: Print succ()
 * 03 Feb 03 - Mike: Mods for cached dataflow
 * 25 Mar 03 - Mike: Print new operators (opWildIntConst, etc)
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <numeric>      // For accumulate
#include <algorithm>    // For std::max()
#include <map>          // In decideType()
#include <sstream>      // Yes, you need gcc 3.0 or better
#include "types.h"
#include "dataflow.h"
#include "cfg.h"
#include "exp.h"
#include "register.h"
#include "rtl.h"        // E.g. class ParamEntry in decideType()
#include "proc.h"
#include "prog.h"
#include "operstrings.h"// Defines a large array of strings for the
                        // createDotFile functions. Needs -I. to find it
#include "util.h"

/*==============================================================================
 * FUNCTION:        Const::Const etc
 * OVERVIEW:        Constructors
 * PARAMETERS:      As required
 * RETURNS:         <nothing>
 *============================================================================*/

// Derived class constructors

Const::Const(int i)     : Exp(opIntConst)   {u.i = i;}
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

TypedExp::TypedExp() : Unary(opTypedExp), type(NULL) {}
TypedExp::TypedExp(Exp* e1) : Unary(opTypedExp, e1), type(NULL) {}
TypedExp::TypedExp(Type* ty, Exp* e1) : Unary(opTypedExp, e1),
    type(ty) {}
TypedExp::TypedExp(TypedExp& o) : Unary(opTypedExp)
{
    subExp1 = o.subExp1->clone();
    type = o.type->clone();
}

AssignExp::AssignExp() : Binary(opAssignExp), size(32) {}
AssignExp::AssignExp(Exp* lhs, Exp* rhs) : Binary(opAssignExp, lhs, rhs), size(32)
{ 
    if (lhs->getOper() == opTypedExp) { 
        size = ((TypedExp*)lhs)->getType()->getSize(); 
    } 
}
AssignExp::AssignExp(int sz, Exp* lhs, Exp* rhs) : Binary(opAssignExp, lhs, rhs), size(sz) { }
AssignExp::AssignExp(AssignExp& o) : Binary(opAssignExp), size(o.size)
{
    subExp1 = o.subExp1->clone();
    subExp2 = o.subExp2->clone();
}

FlagDef::FlagDef(Exp* params, RTL* rtl)
    : Unary(opFlagDef, params), rtl(rtl) {}

UsesExp::UsesExp(Exp* e, Statement* def) : Unary(opSubscript, e) {
    stmtSet.insert(def);
}
UsesExp::UsesExp(Exp* e) : Unary(opSubscript, e) {
}
UsesExp::UsesExp(UsesExp& o) : Unary(o) {
    stmtSet = o.stmtSet;
}

/*==============================================================================
 * FUNCTION:        Unary::~Unary etc
 * OVERVIEW:        Destructors.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
Unary::~Unary() {
    // Remember to delete all children
    if (subExp1 != 0) delete subExp1;
}
Binary::~Binary() {
    if (subExp2 != 0) delete subExp2;
    // Note that the first pointer is destructed in the Exp1 destructor
}
Ternary::~Ternary() {
    if (subExp3 != 0) delete subExp3;
}
FlagDef::~FlagDef() {
    delete rtl;
}

/*==============================================================================
 * FUNCTION:        Unary::setSubExp1 etc
 * OVERVIEW:        Set requested subexpression; 1 is first
 * PARAMETERS:      Pointer to subexpression to set
 * NOTE:            If an expression already exists, it is deleted
 * RETURNS:         <nothing>
 *============================================================================*/
void Unary::setSubExp1(Exp* e) {
    if (subExp1 != 0) delete subExp1;
    subExp1 = e;
}
void Binary::setSubExp2(Exp* e) {
    if (subExp2 != 0) delete subExp2;
    subExp2 = e;
}
void Ternary::setSubExp3(Exp* e) {
    if (subExp3 != 0) delete subExp3;
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
    if (type) delete type;
    type = ty;
}

int AssignExp::getSize()
{
    return size;
}
void AssignExp::setSize(int sz)
{
    size = sz;
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
 * NOTE:            This (enclosing) expression is deleted
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to the requested subexpression
 *============================================================================*/
Exp* Unary::becomeSubExp1() {
    Exp* res = subExp1;
    subExp1 = 0;            // Change pointer to become NULL
    delete this;            // Suicide!
    return res;
}
Exp* Binary::becomeSubExp2() {
    Exp* res = subExp2;
    subExp2 = 0;            // Change pointer to become NULL
    delete this;            // Suicide!
    return res;
}
Exp* Ternary::becomeSubExp3() {
    Exp* res = subExp3;
    subExp3 = 0;            // Change pointer to become NULL
    delete this;            // Suicide!
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
Exp* AssignExp::clone() {
    AssignExp* c = new AssignExp(size, subExp1->clone(), subExp2->clone());
    return c;
}
Exp* UsesExp::clone() {
    UsesExp* c = new UsesExp(subExp1->clone());
    c->stmtSet = stmtSet;
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
        default: std::cerr << "Operator== invalid operator " << operStrings[op]
                   << std::endl;
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
bool AssignExp::operator==(const Exp& o) const {
    if (op == opWild) return true;
    if (((AssignExp&)o).op == opWild) return true;
    if (((AssignExp&)o).op != opAssignExp) return false;
    if (size != ((AssignExp&)o).size) return false;
    return *((Binary*)this)->getSubExp1() == *((Binary&)o).getSubExp1() &&
           *((Binary*)this)->getSubExp2() == *((Binary&)o).getSubExp2();
}

bool UsesExp::operator==(const Exp& o) const {
    if (op == opWild) return true;
    if (((UsesExp&)o).op == opWild) return true;
    if (((UsesExp&)o).op != opSubscript) return false;
    if (!( *subExp1 == *((UsesExp&)o).subExp1)) return false;
    return stmtSet == ((UsesExp&)o).stmtSet;
}

// Compare, ignoring subscripts
bool Exp::operator*=(const Exp& o) const {
    const Exp* one = this;
    if (op == opSubscript)
        one = ((UsesExp*)this)->getSubExp1();
    Exp* two = const_cast<Exp*>(&o);
    if (two->isSubscript())
        two = ((UsesExp*)two)->getSubExp1();
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
        default: std::cerr << "Operator< invalid operator " << operStrings[op]
                   << std::endl;
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

bool AssignExp::operator<  (const Exp& o) const {        // Type sensitive
    if (op < o.getOper()) return true;
    if (op > o.getOper()) return false;
    if (size < ((AssignExp&)o).size) return true;
    if (((AssignExp&)o).size < size) return false;
    if (*subExp1 < *((Binary&)o).getSubExp1()) return true;
    if (*((Binary&)o).getSubExp1() < *subExp1) return false;
    return *subExp2 < *((Binary&)o).getSubExp2();
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
            std::cerr << "Const::print invalid operator " << operStrings[op] <<
              std::endl;
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

        default:
            break;
    }

    // Ordinary infix operators. Emit parens around the binary
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
        case opLessUns: os << " <=u ";break;
        case opGtrUns:  os << " >u "; break;
        case opLessEqUns:os << " <=u ";break;
        case opGtrEqUns: os << " >=u ";break;
        case opUpper:   os << " GT "; break;
        case opLower:   os << " LT "; break;
        case opShiftL:  os << " << "; break;
        case opShiftR:  os << " >> "; break;
        case opShiftRA: os << " >>A "; break;
        case opRotateL: os << " rl "; break;
        case opRotateR: os << " rr "; break;
        case opRotateLC: os << " rlc "; break;
        case opRotateRC: os << " rrc "; break;

        default:
            std::cerr << "Binary::print invalid operator " << operStrings[op]
              << std::endl;
            assert(0);
    }

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
        case opAFP:     os << "%afp";  break;
        case opAGP:     os << "%agp";  break;
        case opWild:    os << "WILD";  break;
        case opAnull:   os << "%anul"; break;
        case opFpush:   os << "FPUSH"; break;
        case opFpop:    os << "FPOP";  break;
        case opWildMemOf:os<< "m[WILD]"; break;
        case opWildRegOf:os<< "r[WILD]"; break;
        case opWildAddrOf:os<< "a[WILD]"; break;
        case opWildIntConst:os<<"WILDINT"; break;
        case opNil:     break;
        default:
            std::cerr << "Terminal::print invalid operator " << operStrings[op]
              << std::endl;
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
        case opRegOf:   case opMemOf:
        case opAddrOf:  case opVar:
            switch (op) {
                case opRegOf: os << "r["; break;
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
        default:
            std::cerr << "Unary::print invalid operator " << operStrings[op] <<
              std::endl;
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
            p1->print(os, withUses); os << ",";
            p2->print(os, withUses); os << ",";
            p3->print(os, withUses); os << ")";
            return;
        default:
            break;
    }
    // Else must be ?: or @ (traditional ternary operators)
    p1->printr(os, withUses);
    if (op == opTern) {
        os << " ? ";
        p2->printr(os, withUses);
        os << " : ";        // Need wide spacing here
        p3->print(os, withUses);
    } 
    else if (op == opAt) {
            os << "@";
            p2->printr(os, withUses);
            os << ":";
            p3->printr(os, withUses);
    } else {
        std::cerr << "Ternary::print invalid operator " << operStrings[op] <<
          std::endl;
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
// AssignExp //
//  //  //  //
void AssignExp::print(std::ostream& os, bool withUses) {
    os << "*" << std::dec << size << "* ";
    Exp* p1 = ((Binary*)this)->getSubExp1();
    p1->print(os, withUses);
    os << " := ";
    Exp* p2 = ((Binary*)this)->getSubExp2();
    p2->print(os, withUses);
}

void AssignExp::getDefinitions(LocationSet &defs) {
    defs.insert(getLeft());
}


//  //  //  //
// UsesExp  //
//  //  //  //
void UsesExp::print(std::ostream& os, bool withUses) {
    subExp1->print(os, withUses);
    if (withUses) {
        os << "{";
        stmtSet.printNums(os);
        os << "}";
    }
}

/*==============================================================================
 * FUNCTION:        Exp::prints
 * OVERVIEW:        Print to a static string (for debugging)
 * PARAMETERS:      <none>
 * RETURNS:         Address of the static buffer
 *============================================================================*/
static char debug_buffer[200];
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
        std::cerr << "Could not open " << name << " to write dotty file\n";
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
// AssignExp //
//  //  //  //
void AssignExp::appendDotFile(std::ofstream& of) {
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    of << "opAssignExp\\n0x" << std::hex << (int)this << " | ";
    of << size << " | <p1>";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    of << "e" << std::hex << (int)this << ":p1->e" << (int)subExp1 << ";\n";
    subExp2->appendDotFile(of);
    of << "e" << std::hex << (int)this << ":p1->e" << (int)subExp2 << ";\n";
}


//  //  //  //
//  FlagDef //
//  //  //  //
void FlagDef::appendDotFile(std::ofstream& of) {
    of << "e" << std::hex << (int)this << " [shape=record,label=\"{";
    of << "opFlagDef \\n0x" << std::hex << (int)this << "| ";
    // Display the RTL as "RTL <r1> <r2>..." vertically (curly brackets)
    of << "{ RTL ";
    int n = rtl->getNumExp();
    for (int i=0; i < n; i++)
        of << "| <r" << std::dec << i << "> ";
    of << "} | <p1> }\"];\n";
    subExp1->appendDotFile(of);
    of << "e" << std::hex << (int)this << ":p1->e" << (int)subExp1 << ";\n";
}

/*==============================================================================
 * FUNCTION:        Exp::isAssign
 * OVERVIEW:        Returns true if the expression is typed assignment
 * PARAMETERS:      <none>
 * RETURNS:         True if matches
 *============================================================================*/
bool Exp::isAssign()
{
    Exp* sub;
    assert(!((op == opTypedExp) &&
      ((sub = ((Binary*)this)->getSubExp1()) != 0) &&
      (sub->getOper() == opAssignExp)));
    return op == opAssignExp;
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
 * FUNCTION:        Exp::isFlagAssgn
 * OVERVIEW:        Returns true if the expression is an asignment to the
 *                    abstract flags location
 * PARAMETERS:      None
 * RETURNS:         True if matches
 *============================================================================*/
bool Exp::isFlagAssgn() {
    if (op != opAssignExp) return false;
    if (getSubExp1()->getOper() != opFlags) return false;
    if (getSubExp2()->getOper() != opFlagCall) return false;
    return true;
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
    compare = (*search *= *pSrc);           // Note: subscript insensitive
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
 *                  is already deleted.
 * NOTE:            Replacements are cloned. Caller to delete search and replace
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
        if (*pp) delete *pp;        // Delete any existing
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
 *                    each found, return a pointer to the pointer to the matched
 *                    expression in results
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

bool AssignExp::searchAndReplace(Exp* search, Exp* replace) {
    bool change = false;
    Exp *e = searchReplaceAll(search, replace, change);
    assert(e == this);
    return change;
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
 * NOTE:            Expressions are NOT cloned. Therefore, do not delete the
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
        case opAssignExp:
            p1 = ((Binary*)this)->getSubExp1();
            p2 = ((Binary*)this)->getSubExp1();
            p1->partitionTerms(positives, negatives, integers, negate);
            p2->partitionTerms(positives, negatives, integers, negate);
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
 * NOTE:            User must delete result
 * PARAMETERS:      <none>
 * RETURNS:         Ptr to the simplified expression
 *============================================================================*/
Exp* Unary::simplifyArith()
{
    if (op == opMemOf || op == opRegOf) {
        // assume we want to simplify the subexpression
        subExp1 = subExp1->simplifyArith();
    }
    return this;            // Else, do nothing
}

Exp* AssignExp::simplifyArith() {
    subExp1 = subExp1->simplifyArith();
    subExp2 = subExp2->simplifyArith();
    return this;
}

Exp* Ternary::simplifyArith() {
    subExp1 = subExp1->simplifyArith();
    subExp2 = subExp2->simplifyArith();
    subExp3 = subExp3->simplifyArith();
    return this;
}

Exp* Binary::simplifyArith() {
    if ((op != opPlus) && (op != opMinus)) {
        subExp1 = subExp1->simplifyArith();
        subExp2 = subExp2->simplifyArith();
        return this;
    }

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
            return new Binary(opPlus,
                Exp::Accumulate(positives), new Const(sum));
        }
    }
    // Some positives, some negatives
    if (sum == 0) {
        // positives - negatives
        return new Binary(opMinus, Exp::Accumulate(positives),
            Exp::Accumulate(negatives));
    }
    // General case: some positives, some negatives, a sum
    return new Binary(opPlus,
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
 * NOTE:            User must delete result
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
    if (!(*res == *save)) std::cout << "simplified " << save << " to " << res
      << "\n";
    delete save;
#endif
    return res;
}

/*==============================================================================
 * FUNCTION:        Unary::simplify etc
 * OVERVIEW:        Do the work of simplification
 * NOTE:            User must delete result
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
                // it deletes this!
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
            subExp1 = subExp1->simplifyArith();  // probably bad
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
            case opAnd:         k1 = k1 && k2; break;
            case opOr:          k1 = k1 || k2; break;
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
            delete res;
            res = new Const(k1);
            bMod = true;
            return res;
        }
    }

    if (((op == opBitXor) || (op == opMinus)) && (*subExp1 == *subExp2)) {
        // x ^ x or x - x: result is zero
        delete res;
        res = new Const(0);
        bMod = true;
        return res;
    }

    // turn a - b into a + -b
    // Counts as a change, in case b happens to be an integer constant
    // (then neg int 99 -> int -99)
    if (op == opMinus) {
        subExp2 = new Unary(opNeg, subExp2);
        op = opPlus;
        opSub2 = opNeg;
        bMod = true;
    }

    // Might want to commute to put an integer constant on the RHS
    // Later simplifications can rely on this (ADD other ops as necessary)
    if (opSub1 == opIntConst && 
        (op == opPlus || op == opMult   || op == opMults || op == opBitOr) ||
         op == opOr   || op == opBitAnd || op == opAnd) {
        ((Binary*)res)->commute();
        // Swap opSub1 and opSub2 as well
        OPER t = opSub1;
        opSub1 = opSub2;
        opSub2 = t;
        // This is not counted as a modification
    }

    // Check for exp + 0  or  exp - 0  or  exp | 0 or exp OR 0
    int k;
    if ((op == opPlus || op == opMinus || op == opBitOr || (op == opOr)) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0) {
        res = ((Unary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }
       
    // Check for exp * 0  or exp & 0  or exp AND 0 
    if ((op == opMult || op == opMults || op == opBitAnd || (op == opAnd)) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0) {
        delete res;
        res = new Const(0);
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
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() != 0) {
        res = ((Unary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    // NOTE: this simplification used to be around the other way, i.e.
    // x + -4 was changed to x - 4.
    // However, this is inconsistent with what happens with
    // simplifyArith(). Also, it makes more sense to do it the
    // other way, i.e. change x - 4 to x + -4 (only have to check for
    // opPlus, not opPlus and opMinus)
    // Change x - 4 to x + -4 (and x - -5 to x + 5)
    if ((op == opMinus) && (opSub2 == opIntConst)) {
        res->setOper(opPlus);
        k = ((Const*)subExp2)->getInt();
        ((Const*)subExp2)->setInt(-k);
        bMod = true;
        return res;
    }

    // Check for [exp] << k where k is a positive integer const
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
        delete e;
        subExp2 = new Unary(opNeg, subExp2);
    }

    // Check for (x + y) compare 0, becomes x compare -y
    if (isComparison() &&
        opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0 && 
        opSub1 == opPlus) {
        delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        delete b;
        subExp2 = new Unary(opNeg, subExp2);
        bMod = true;
        return res;
    }
*/
    // Check for (x == y) == 1, becomes x == y
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 1 && opSub1 == opEquals) {
        delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        delete b;
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
            delete subExp2;
            subExp2 = b->subExp2;
            ((Const*)subExp2)->setInt(-((Const*)subExp2)->getInt());
            b->subExp2 = 0;
            subExp1 = b->subExp1;
            b->subExp1 = 0;
            delete b;
            bMod = true;
            return res;
        }
    }

    // Check for (x == y) == 0, becomes x != y
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 0 && opSub1 == opEquals) {
        delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        delete b;
        bMod = true;
        res->setOper(opNotEqual);
        return res;
    }

    // Check for (x == y) != 1, becomes x != y
    if (op == opNotEqual && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 1 && opSub1 == opEquals) {
        delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        delete b;
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
        delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        delete b;
        bMod = true;
        res->setOper(opLessEq);
        return res;
    }

    // Check for (x >u y) == 0, becomes x <=u y
    if (op == opEquals && opSub2 == opIntConst &&
        ((Const*)subExp2)->getInt() == 0 && opSub1 == opGtrUns) {
        delete subExp2;
        Binary *b = (Binary*)subExp1;
        subExp2 = b->subExp2;
        b->subExp2 = 0;
        subExp1 = b->subExp1;
        b->subExp1 = 0;
        delete b;
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
    OPER opSub1 = subExp1->getOper();

    assert(opSub1 != opAssignExp);

    subExp1 = subExp1->polySimplify(bMod);
    return res;
}

Exp* AssignExp::polySimplify(bool& bMod) {
    Exp *res = this;
    //OPER opSub1 = subExp1->getOper();
    //OPER opSub2 = subExp2->getOper();

    subExp1 = subExp1->polySimplify(bMod);
    subExp2 = subExp2->polySimplify(bMod);
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

Exp* TypedExp::simplifyAddr() {
    subExp1 = subExp1->simplifyAddr();
    return this;
} 

Exp* AssignExp::simplifyAddr() {
    subExp1 = subExp1->simplifyAddr();
    subExp2 = subExp2->simplifyAddr();
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

/*==============================================================================
 * FUNCTION:        Exp::deserialize / Const::serialize
 * OVERVIEW:        Serialize the expression
 * PARAMETERS:      output buffer, output length
 * RETURNS:         true if successful
 *============================================================================*/
Exp *Exp::deserialize(std::istream &inf)
{
    Exp *e = NULL;
    char ch;
    loadValue(inf, ch, false);
    OPER op;
    int iop;
    loadValue(inf, iop, false);
    op = (OPER)iop;

    switch(ch) {
        case 'C':
            {
                int i; double d; std::string s;
                switch(op) {
                    case opIntConst:
                        loadValue(inf, i, false);
                        e = new Const(i);
                        break;
                    case opFltConst:
                        loadValue(inf, d, false);
                        e = new Const(d);
                        break;
                    case opStrConst:
                        loadString(inf, s);
                        e = new Const(strdup(s.c_str()));
                        break;
                    default:
                        std::cerr << "WARNING: unknown const expression type, "
                            "ignoring, data will be lost!" << std::endl;
                }
            }
            break;
        case 't':
            e = new Terminal(op);
            break;
        case 'U':
            {
                Exp *e1 = deserialize(inf);
                e = new Unary(op, e1);
            }
            break;
        case 'B':
            {
                Exp *e1 = deserialize(inf);
                Exp *e2 = deserialize(inf);
                e = new Binary(op, e1, e2);
            }
            break;
        case 'T':
            {
                Exp *e1 = deserialize(inf);
                Exp *e2 = deserialize(inf);
                Exp *e3 = deserialize(inf);
                e = new Ternary(op, e1, e2, e3);
            }
            break;
        case 'y':
            {
                Type *t = Type::deserialize(inf);
                Exp *e1 = deserialize(inf);
                e = new TypedExp(t, e1);
            }
            break;
        case 'A':
            {
                int sz;
                loadValue(inf, sz, false);
                Exp *e1 = deserialize(inf);
                Exp *e2 = deserialize(inf);
                e = new AssignExp(sz, e1, e2);
            }
            break;
        case 'F':
            {
                Exp *e1 = deserialize(inf);
                RTL *r = RTL::deserialize(inf);
                e = new FlagDef(e1, r);
            }
            break;
        default:
            std::cerr << "WARNING: unknown expression type, ignoring, "
                "data will be lost!" << std::endl;
    }

    if (e) {
        assert(loadFID(inf) == FID_EXP_END);
        assert(loadLen(inf) == 0);
    }

    return e;
}

bool Const::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 'C';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);
    switch(op) {
        case opIntConst:
            saveValue(ouf, u.i, false);
            break;
        case opFltConst:
            saveValue(ouf, u.d, false);
            break;
        case opStrConst:
            {
                std::string s(u.p);
                saveString(ouf, s);
            }
            break;
        default:
            // add a new case
            assert(false);
    }

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool Terminal::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 't';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool Unary::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 'U';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);

    int l;
    subExp1->serialize(ouf, l);

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool Binary::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 'B';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);

    int l;
    subExp1->serialize(ouf, l);
    subExp2->serialize(ouf, l);

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool Ternary::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 'T';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);

    int l;
    subExp1->serialize(ouf, l);
    subExp2->serialize(ouf, l);
    subExp3->serialize(ouf, l);

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool TypedExp::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 'y';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);

    int l;
    type->serialize(ouf, l);
    subExp1->serialize(ouf, l);

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool AssignExp::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 'A';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);

    saveValue(ouf, size, false);
    int l;
    subExp1->serialize(ouf, l);
    subExp2->serialize(ouf, l);

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool FlagDef::serialize(std::ostream &ouf, int &len)
{
    std::streampos st = ouf.tellp();

    char ch = 'F';
    saveValue(ouf, ch, false);
    saveValue(ouf, (int)op, false);

    int l;
    subExp1->serialize(ouf, l);
    rtl->serialize(ouf, l);

    saveFID(ouf, FID_EXP_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

void AssignExp::killDef(StatementSet &reach) {
    StatementSet kills;
    StmtSetIter it;
    for (Statement* s = reach.getFirst(it); s; s = reach.getNext(it)) {
        if (s->getLeft() == NULL) continue;     // Should never happen?
        bool isKilled = false;
        // Note: *= is "subscript insensitive" comparison. If attempt to
        // redo global dataflow in SSA form, may need to change this
        if (*s->getLeft() *= *subExp1)
            isKilled = true;
        // The below is NOT necessarily conservative!
        // In fact, it prevents hello world from working, so is commmented
        // out for now:
        //isKilled |= mayAlias(s->getLeft(), subExp1, getSize());
        if (isKilled)
            kills.insert(s);
    }
    reach.makeDiff(kills);
}

// Liveness is killed by a definition
void AssignExp::killLive(LocationSet &live) {
    if (subExp1 == NULL) return;
    LocSetIter it;
    for (Exp* loc = live.getFirst(it); loc; loc = live.getNext(it)) {
        // MVE: do we need to consider aliasing?
        if (*loc == *subExp1)
            live.remove(loc);
    }
}

// Deadness is killed by a use
void AssignExp::killDead(LocationSet &dead) {
    LocationSet uses;
    addUsedLocs(uses);
    dead.makeDiff(uses);
}

// MVE: I don't think that this will be needed any more
void AssignExp::getDeadStatements(StatementSet &dead)
{
    StatementSet reach;
    getReachIn(reach, 2);
    StmtSetIter it;
    for (Statement* s = reach.getFirst(it); s; s = reach.getNext(it)) {
        if (s->getLeft() == NULL) continue;
        bool isKilled = false;
        if (*s->getLeft() == *subExp1)
            isKilled = true;
        if (s->getLeft()->isMemOf() && subExp1->isMemOf())
            isKilled = true; // might alias, very "conservative"
        if (isKilled && s->getNumUsedBy() == 0)
        dead.insert(s);
    }
}

// update type for expression
Type *AssignExp::updateType(Exp *e, Type *curType) {
    return curType;
}

bool AssignExp::usesExp(Exp *e) {
    Exp *where = 0;
    return (subExp2->search(e, where) || (subExp1->isMemOf() && 
        ((Unary*)subExp1)->getSubExp1()->search(e, where)));
}

void AssignExp::doReplaceUse(Statement *def) {
    Exp *defLeft = def->getLeft();
    Exp *defRight = def->getRight();
    assert(defLeft);
    assert(defRight);
    bool changeright = false;
    subExp2 = subExp2->searchReplaceAll(defLeft, defRight, changeright);
    bool changeleft = false;
    Exp* baseSub1 = subExp1;
    if (subExp1->isSubscript()) baseSub1 = ((UsesExp*)subExp1)->getSubExp1();
    if (baseSub1->isMemOf()) {
        Exp *e = baseSub1->getSubExp1()->clone();
        e = e->searchReplaceAll(defLeft, defRight, changeleft);
        baseSub1->setSubExp1(e);
    }
//    assert(changeright || changeleft);
    // simplify the expression
    subExp2 = subExp2->simplifyArith();
    subExp1 = subExp1->simplifyArith();
    simplify();
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
Exp* Unary::fixSuccessor() {
    bool change;
    Exp* result;
    // Assume only one successor function in any 1 assignment
    if (search(&succRegOf, result)) {
        // Result has the matching expression, i.e. succ(r[K])
        Exp* sub1 = ((Unary*)result)->getSubExp1();
        assert(sub1->getOper() == opRegOf);
        Exp* sub2 = ((Unary*)sub1)->getSubExp1();
        assert(sub2->getOper() == opIntConst);
        // result    sub1   sub2
        // succ(      r[   Const K  ])
        // Note: we need to clone the r[K] part, since it will be deleted as
        // part of the searchReplace below
        Unary* replace = (Unary*)sub1->clone();
        Const* c = (Const*)replace->getSubExp1();
        c->setInt(c->getInt()+1);       // Do the increment
        Exp* res = searchReplace(result, replace, change);
        return res;
    }
    return this;
}
    
void AssignExp::processConstants(Prog *prog)
{
    // TODO
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

/*==============================================================================
 * FUNCTION:        AssignExp::addUsedLocs
 * OVERVIEW:        Add all locations (registers or memory) used by this
 *                    assignment
 * PARAMETERS:      used: ref to a LocationSet to insert the used locations into
 * RETURNS:         nothing
 *============================================================================*/
void AssignExp::addUsedLocs(LocationSet& used) {
    Exp* left = getSubExp1();
    Exp* right = getSubExp2();
    right->addUsedLocs(used);
    Exp* baseLeft = left;
    if (left->isSubscript())        // Should always be
        baseLeft = ((UsesExp*)left)->getSubExp1();
    if (baseLeft->isMemOf()) {
        // We also use any expr like m[exp] on the LHS (but not the outer m[])
        Exp* baseLeftChild = ((Unary*)baseLeft)->getSubExp1();
        baseLeftChild->addUsedLocs(used);
    }
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
    if (op == opPC || op == opFlags)
        used.insert(clone());
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

void UsesExp::addUsedLocs(LocationSet& used) {
    used.insert(clone());           // We want to see these
    if (subExp1->isMemOf()) {
        Exp* grandChild = ((Unary*)subExp1)->getSubExp1();
        grandChild->addUsedLocs(used);
    }
}

Exp* Exp::addSubscript(Statement* def) {
        return new UsesExp(this, def);
}

/*==============================================================================
 * FUNCTION:        Unary::updateRefs etc
 * OVERVIEW:        Update the references ("uses" (rhymes with "fuses"))
 *                    information inherent in each component of an expression
 * PARAMETERS:      defs: ref to a StatementSet of statements reaching this
 * RETURNS:         <nothing>
 *============================================================================*/
//  //  //  //
//  Unary   //
//  //  //  //
Exp* Unary::updateRefs(StatementSet& defs, int memDepth) {
    // Only consider expressions that are of the correct "memory nesting depth"
    Exp* res = this;
    if (getMemDepth() == memDepth) {
        StmtSetIter it;
        for (Statement* s = defs.getFirst(it); s; s = defs.getNext(it)) {
            Exp* left = s->getLeft();
            assert(left);           // A definition must have a left
            if (*left == *this)
                // Need to subscript.
                // Note: several defs could reach, so don't return yet
                res = res->addSubscript(s);
        }
    }
    // Recurse (in case a memof, addrof etc)
    // Recurse even if memory depths don't match; we want to subscript all
    // the parameters to m[]'s
    subExp1 = subExp1->updateRefs(defs, memDepth);
    return res;
}

//  //  //  //
//  Binary  //
//  //  //  //
Exp* Binary::updateRefs(StatementSet& defs, int memDepth) {
    subExp1 = subExp1->updateRefs(defs, memDepth);
    subExp2 = subExp2->updateRefs(defs, memDepth);
    return this;
}

//  //  //  //  //
//  AssignExp   //
//  //  //  //  //
Exp* AssignExp::updateRefs(StatementSet& defs, int memDepth) {
    // No need to test for equality to left
    // However, need to update aa iff LHS is m[aa]
    if (subExp1->isMemOf())
        // Beware. Consider left == m[r[28]]; subExp1 is the same.
        // If we call subExp1->updateRefs, we will double subscript our
        // LHS (violating a basic property of SSA form)
        // If we call left->updateRefs, we would get a
        // subscript of a subscript, also not what we want!
        // Don't call setSubExp1 either, since it deletes the old
        // expression (old expression is always needed)
        ((Unary*)subExp1)->setSubExp1ND(subExp1->getSubExp1()->
          updateRefs(defs, memDepth));
    subExp2 = subExp2->updateRefs(defs, memDepth);
    return this;
}

//  //  //  //
//  UsesExp //
//  //  //  //
Exp* UsesExp::updateRefs(StatementSet& defs, int memDepth) {
    // The left of any definition can't be a UsesExp
    // (LHS not subscripted any more)
    subExp1 = subExp1->updateRefs(defs, memDepth);
    return this;
}

//  //  //  //
// Ternary  //
//  //  //  //
Exp* Ternary::updateRefs(StatementSet& defs, int memDepth) {
    subExp1 = subExp1->updateRefs(defs, memDepth);
    subExp2 = subExp2->updateRefs(defs, memDepth);
    subExp3 = subExp3->updateRefs(defs, memDepth);
    return this;
}

//  //  //  //
// Terminal //
//  //  //  //
Exp* Terminal::updateRefs(StatementSet& defs, int memDepth) {
    if (memDepth != 0) return this;
    StmtSetIter it;
    Exp* res = this;
    for (Statement* s = defs.getFirst(it); s; s = defs.getNext(it)) {
        Exp* left = s->getLeft();
        assert(left);           // A definition must have a left
        if (*left == *this)
            res = res->addSubscript(s);
    }
    return res;
}

//
// From SSA form
//

Exp* UsesExp::fromSSA(igraph& ig) {
    // FIXME: Need to check if the argument is a memof, and if so
    // deal with that specially (e.g. global)
    // Check to see if it is in the map
    igraph::iterator it = ig.find(this);
    if (it == ig.end())
        // It is not in the map. Just remove the opSubscript
        return becomeSubExp1();
    else {
        // It is in the map. Delete the current expression, and replace
        // with a new local
        std::ostringstream os;
        os << "temp" << ig[this];
        std::string name = os.str();
        delete this;
        return new Unary(opLocal, new Const(strdup(name.c_str())));
    }
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

void AssignExp::fromSSAform(igraph& ig) {
    subExp1 = subExp1->fromSSA(ig);
    subExp2 = subExp2->fromSSA(ig);
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

    

// Might be a useful framework for various tests one day
void Exp::check() {
    Ternary* t = dynamic_cast<Ternary*>(this);
    if (t) {
        t->getSubExp1()->check();
        t->getSubExp2()->check();
        t->getSubExp3()->check();
    } else {
        Binary* b = dynamic_cast<Binary*>(this);
        if (b) {
            b->getSubExp1()->check();
            b->getSubExp2()->check();
        } else {
            Unary* u = dynamic_cast<Unary*>(this);
            if (u) {
                if (op == opSubscript) {
                    UsesExp* u = dynamic_cast<UsesExp*>(this);
                    if (u) std::cerr << ".";
                    else
                        std::cerr << "Here it is!!!\n";
                }
                u->getSubExp1()->check();
            } else {
                Terminal* t = dynamic_cast<Terminal*>(this);
                if (t == NULL) {
                    Const* c = dynamic_cast<Const*>(this);
                    if (c == NULL)
                        std::cerr << "None of the above: " << this << "\n";
                }
            }
        }
    }
}

// A helper file for comparing Exp*'s sensibly
bool lessExpStar::operator()(const Exp* x, const Exp* y) const {
    return (*x < *y);       // Compare the actual Exps
}

bool lessTI::operator()(const Exp* x, const Exp* y) const {
    return (*x << *y);      // Compare the actual Exps
}
