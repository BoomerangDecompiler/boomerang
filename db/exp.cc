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
 */


#include <numeric>      // For accumulate
#include <map>          // In decideType()
#include <sstream>      // Yes, you need gcc 3.0 or better
#include "exp.h"
#include "rtl.h"        // E.g. class ParamEntry in decideType()
#include "prog.h"
#include "operstrings.h"// Defines a large array of strings for the
                        // createDotFile functions. Needs -I. to find it
// Global (shame)
extern Prog prog;

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
Const::Const(ADDRESS a) : Exp(opAddrConst)  {u.a = a;}
// Note: need something special for opCodeAddr

// Copy constructor
Const::Const(Const& o) : Exp(o.op) {u = o.u;}

Terminal::Terminal(OPER op) : Exp(op) {}
Terminal::Terminal(Terminal& o) : Exp(o.op) {}      // Copy constructor

Unary::Unary(OPER op)
    : Exp(op)
{
    subExp1 = 0;        // Initialise the pointer
}
Unary::Unary(OPER op, Exp* e)
    : Exp(op)
{
    subExp1 = e;        // Initialise the pointer
}
Unary::Unary(Unary& o)
    : Exp(o.op)
{
    subExp1 = o.subExp1->clone();
}

Binary::Binary(OPER op)
    : Unary(op)
{
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

TypedExp::TypedExp() : Unary(opTypedExp) {}
TypedExp::TypedExp(Exp* e1) : Unary(opTypedExp, e1) {}
TypedExp::TypedExp(const Type& ty, Exp* e1) : Unary(opTypedExp, e1),
    type(ty) {}
TypedExp::TypedExp(TypedExp& o) : Unary(opTypedExp), type(o.type)
{
    subExp1 = o.subExp1->clone();
}

FlagDef::FlagDef(Exp* params, RTL* rtl)
    : Unary(opFlagDef, params), rtl(rtl) {}

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
void Unary::setSubExp1(Exp* e)
{
    if (subExp1 != 0) delete subExp1;
    subExp1 = e;
}
void Binary::setSubExp2(Exp* e)
{
    if (subExp2 != 0) delete subExp2;
    subExp2 = e;
}
void Ternary::setSubExp3(Exp* e)
{
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

Type& TypedExp::getType()
{
    return type;
}
void TypedExp::setType(Type& ty)
{
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
Exp* Const::clone()
{
    return new Const(*this);
}
Exp* Terminal::clone()
{
    return new Terminal(*this);
}
Exp* Unary::clone()
{
    Unary* c = new Unary(op);
    c->subExp1 = subExp1->clone();
    return c;
}
Exp* Binary::clone()
{
    Binary* c = new Binary(op);
    c->subExp1 = subExp1->clone();
    c->subExp2 = subExp2->clone();
    return c;
}

Exp* Ternary::clone()
{
    Ternary* c = new Ternary(op);
    c->subExp1 = subExp1->clone();
    c->subExp2 = subExp2->clone();
    c->subExp3 = subExp3->clone();
    return c;
}
Exp* TypedExp::clone()
{
    TypedExp* c = new TypedExp(type, subExp1->clone());
    return c;
}

/*==============================================================================
 * FUNCTION:        Const::operator==() etc
 * OVERVIEW:        Virtual function to compare myself for equality with
 *                  another Exp
 * NOTE:            The test for a wildcard is only with this object, not
 *                    the other object (o). So when searching and there could
 *                    be wildcards, use search == *this not *this == search
 * PARAMETERS:      Ref to other Exp
 * RETURNS:         True if equal
 *============================================================================*/
bool Const::operator==(const Exp& o) const
{
    if (op == opWild) return true;
    if (op != ((Const&)o).op) return false;
    switch (op) {
        case opIntConst: return u.i == ((Const&)o).u.i;
        case opFltConst: return u.d == ((Const&)o).u.d;
        case opStrConst: return (strcmp(u.p, ((Const&)o).u.p) == 0);
        case opAddrConst:return u.a == ((Const&)o).u.a;
        default: std::cerr << "Operator== invalid operator " << operStrings[op]
                   << std::endl;
                 assert(0);
    }
    return false;
}
bool Unary::operator==(const Exp& o) const
{
    if (op == opWild) return true;
    if (op != ((Unary&)o).op) return false;
    return *subExp1 == *((Unary&)o).getSubExp1();
}
bool Binary::operator==(const Exp& o) const
{
    if (op == opWild) return true;
    if (op != ((Binary&)o).op) return false;
    if (!( *subExp1 == *((Binary&)o).getSubExp1())) return false;
    return *subExp2 == *((Binary&)o).getSubExp2();
}
bool Ternary::operator==(const Exp& o) const
{
    if (op == opWild) return true;
    if (op != ((Ternary&)o).op) return false;
    if (!( *subExp1 == *((Ternary&)o).getSubExp1())) return false;
    if (!( *subExp2 == *((Ternary&)o).getSubExp2())) return false;
    return *subExp3 == *((Ternary&)o).getSubExp3();
}
bool Terminal::operator==(const Exp& o) const
{
    return ((op == opWild) ||           // Wild matches anything
      (op ==((Terminal&)o).op));
}
bool TypedExp::operator==(const Exp& o) const
{
    if (op == opWild) return true;
    if (op != opTypedExp) return false;
    // This is the strict type version
    if (type != ((TypedExp&)o).type) return false;
    return *((Unary*)this)->getSubExp1() == *((Unary&)o).getSubExp1();
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
        if (type -= ((TypedExp&)o).type) return false;
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
        case opAddrConst:
            return u.a < ((Const&)o).u.a;
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
    if (type < ((TypedExp&)o).type) return true;
    if (((TypedExp&)o).type < type) return false;
    return *subExp1 < *((Unary&)o).getSubExp1();
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
void Const::print(std::ostream& os) {
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
        case opAddrConst: 
            os << "0x" << std::hex << u.a;
            break;
        default:
            std::cerr << "Const::print invalid operator " << operStrings[op] << std::endl;
            assert(0);
    }
}

void Const::printNoQuotes(std::ostream& os) {
    if (op == opStrConst)
        os << u.p;
    else
        print(os);
}

//  //  //  //
//  Binary  //
//  //  //  //
void Binary::printr(std::ostream& os) {
    // The "r" is for recursive: the idea is that we don't want parentheses at
    // the outer level, but a subexpression (recursed from a higher level), we
    // want the parens (at least for standard infix operators)
    switch (op) {
        case opSize:
        case opList:        // Otherwise, you get (a, (b, (c, d)))
        // There may be others
            // These are the noparen cases
            print(os); return;
        default:
            break;
    }
    // Normal case: we want the parens
    // std::ostream::operator<< uses print(), which does not have the parens
    os << "(" << this << ")";
}

void Binary::print(std::ostream& os)
{
    Exp* p1; Exp* p2;
    p1 = ((Binary*)this)->getSubExp1();
    p2 = ((Binary*)this)->getSubExp2();
    // Special cases
    switch (op) {
        case opSize:
            // {size} is printed after the expression
            p2->printr(os); os << "{"; p1->printr(os); os << "}";
            return;
        case opFlagCall:
            // The name of the flag function (e.g. ADDFLAGS) should be enough
            ((Const*)p1)->printNoQuotes(os);
            os << "( "; p2->printr(os); os << " )";
            return;
        case opExpTable:
        case opNameTable:
            if (op == opExpTable)
                os << "exptable(";
            else
                os << "nametable(";
            os << p1 << ", " << p2 << ")";
            return;

        case opAssign:
            p1->print(os); os << " := ";
            p2->print(os);       // Don't want parens here
            return;
        case opList:
            // Because "," is the lowest precedence operator, we don't need
            // printr here. Also, same as UQBT, so easier to test
            p1->print(os);
            if (!p2->isNil())
                os << ", "; 
            p2->print(os);
            return;
        default:
            break;
    }

    // Ordinary infix operators. Emit parens around the binary
    p1->printr(os);
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

    p2->printr(os);

}

//  //  //  //  //
//   Terminal   //
//  //  //  //  //
void Terminal::print(std::ostream& os) {
    switch (op) {
        case opPC:      os << "%pc";   break;
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
void Unary::print(std::ostream& os) {
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
            if (op == opVar) ((Const*)p1)->printNoQuotes(os);
            // Use print, not printr, because this is effectively the top
            // level again (because the [] act as parentheses)
            else p1->print(os);
            os << "]";
            break;

        //  //  //  //  //  //  //
        //    Unary operators   //
        //  //  //  //  //  //  //

        case opNot:     case opLNot:    case opNeg:
                 if (op == opNot)  os << "~";
            else if (op == opLNot) os << "L~";
            else                   os << "-";
            p1->printr(os);
            return;

        case opSignExt:
            p1->printr(os);
            os << "!";          // Operator after expression
            return;

        //  //  //  //  //  //  //  //
        //  Function-like operators //
        //  //  //  //  //  //  //  //

        case opSQRTs: case opSQRTd: case opSQRTq:
        case opSqrt: case opSin: case opCos:
        case opTan: case opArcTan: case opLog2:
        case opLog10: case opLoge: case opMachFtr:
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
                default: break;         // For warning
            }
            p1->printr(os);
            os << ")";
            return;

        //  Misc    //
        case opSgnEx:      // Different because the operator appears last
            p1->printr(os);
            os << "! ";
            return;
        case opTemp:
            // Temp: just print the string, no quotes
        case opParam:
            // Print a more concise form than param["foo"] (just foo)
            ((Const*)p1)->printNoQuotes(os);
            return;
        default:
            std::cerr << "Unary::print invalid operator " << operStrings[op] << std::endl;
            assert(0);
    }
}

//  //  //  //
//  Ternary //
//  //  //  //
void Ternary::printr(std::ostream& os) {
    // The function-like operators don't need parentheses
    switch (op) {
        // The "function-like" ternaries
        case opTruncu:  case opTruncs:  case opZfill:
        case opSgnEx:   case opFsize:   case opItof:
        case opFtoi:    case opFround:  case opOpTable:
            // No paren case
            print(os); return;
        default:
            break;
    }
    // All other cases, we use the parens
    os << "(" << this << ")";
}

void Ternary::print(std::ostream& os) {
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
            p1->print(os); os << ",";
            p2->print(os); os << ",";
            p3->print(os); os << ")";
            return;
        default:
            break;
    }
    // Else must be ?: or @ (traditional ternary operators)
    p1->printr(os);
    if (op == opTern) {
        os << " ? ";
        p2->printr(os);
        os << " : ";        // Need wide spacing here
        p3->print(os);
    } 
    else if (op == opAt) {
            os << "@";
            p2->printr(os);
            os << ":";
            p3->printr(os);
    } else {
        std::cerr << "Ternary::print invalid operator " << operStrings[op] << std::endl;
        assert(0);
    }
}

//  //  //  //
// TypedExp //
//  //  //  //
void TypedExp::print(std::ostream& os) {
    os << "*" << std::dec << type.getSize() << "* ";
    Exp* p1 = ((Ternary*)this)->getSubExp1();
    p1->print(os);
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
        case opCodeAddr:
        case opAddrConst: of << "0x" << std::hex << u.a; break;
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
    of << type.getCtype() << " | <p1>";
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
    return (op == opTypedExp) &&
      ((sub = ((Binary*)this)->getSubExp1()) != 0) &&
      (sub->getOper() == opAssign);
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
    // Should be typed
    if (op != opTypedExp) return 0;
    Exp* sub = ((Unary*)this)->getSubExp1();
    if (sub->getOper() != opGuard) return false;
    return ((Unary*)sub)->getSubExp1();
}

/*==============================================================================
 * FUNCTION:        Exp::doSearch
 * OVERVIEW:        Search for the given subexpression
 * NOTE:            Caller must free the list li after use, but not the
 *                    Exp objects that they point to
 * NOTE:            If the top level expression matches, li will contain search
 * NOTE:            Static function
 * PARAMETERS:      search: ptr to Exp we are searching for
 *                  typeSens: if true, comparisons are type sensitive
 *                  pSrc: ref to ptr to Exp to search. Reason is that we can
 *                    then overwrite that pointer to effect a replacement
 *                  li: list of Exp** where pointers to the matches are found
 *                  once: true if not all occurrences to be found, false for all
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::doSearch(Exp* search, bool typeSens, Exp*& pSrc, std::list<Exp**>& li,
  bool once)
{
    bool compare;
    if (typeSens)
        compare = (*search == *pSrc);       // Consider type
    else
        compare = (*search %= *pSrc);       // Ignore type
    if (compare) {
        li.push_back(&pSrc);                // Success
        if (once)
            return;                         // No more to do
    }
    // Either want to find all occurrences, or did not match at this level
    // Recurse into children
    pSrc->doSearchChildren(search, typeSens, li, once);
}

/*==============================================================================
 * FUNCTION:        Exp::doSearchChildren
 * OVERVIEW:        Search for the given subexpression in all children
 * NOTE:            Virtual function; different implementation for each
 *                  subclass of Exp
 * NOTE:            Will recurse via doSearch
 * PARAMETERS:      search: ptr to Exp we are searching for
 *                  typeSens: if true, comparisons are type sensitive
 *                  li: list of Exp** where pointers to the matches are found
 *                  once: true if not all occurrences to be found, false for all
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::doSearchChildren(Exp* search, bool typeSens, 
  std::list<Exp**>& li, bool once)
{
    return;         // Const and Terminal do not override this
}
void Unary::doSearchChildren(Exp* search, bool typeSens,
  std::list<Exp**>& li, bool once)
{
    subExp1->doSearch(search, typeSens, subExp1, li, once);
}
void Binary::doSearchChildren(Exp* search, bool typeSens,
  std::list<Exp**>& li, bool once)
{
    getSubExp1()->doSearch(search, typeSens, subExp1, li, once);
    if (once && li.size()) return;
    subExp2->doSearch(search, typeSens, subExp2, li, once);
}
void Ternary::doSearchChildren(Exp* search, bool typeSens,
  std::list<Exp**>& li, bool once)
{
    getSubExp1()->doSearch(search, typeSens, subExp1, li, once);
    if (once && li.size()) return;
    getSubExp2()->doSearch(search, typeSens, subExp2, li, once);
    if (once && li.size()) return;
    subExp3->doSearch(search, typeSens, subExp3, li, once);
}
void TypedExp::doSearchChildren(Exp* search, bool typeSens,
  std::list<Exp**>& li, bool once)
{
    subExp1->doSearch(search, typeSens, subExp1, li, once);
}

/*==============================================================================
 * FUNCTION:        Exp::searchReplace
 * OVERVIEW:        Search for the given subexpression, and replace if found
 * NOTE:            If the top level expression matches, search will be changed
 * PARAMETERS:      search:  ptr to Exp we are searching for
 *                  replace: ptr to Exp to replace it with
 *                  change: ref to boolean, set true if a change made
 *                  typeSens: if true, comparisons are type sensitive
 * RETURNS:         True if a change made
 *============================================================================*/
Exp* Exp::searchReplace(Exp* search, Exp* replace, bool& change,
  bool typeSens /* = false */)
{
    return searchReplaceAll(search, replace, change, typeSens, true);
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
 *                  typeSens: if true, comparisons are type sensitive
 * RETURNS:         True if a change made
 *============================================================================*/
Exp* Exp::searchReplaceAll(Exp* search, Exp* replace, bool& change,
  bool typeSens /* = false */, bool once /* = false */ )
{
    std::list<Exp**> li;
    Exp* top = this;        // top may change; that's why we have to return it
    doSearch(search, typeSens, top, li, false);
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
 *                  typeSens: if true, comparisons are type sensitive
 * RETURNS:         True if a match was found
 *============================================================================*/
bool Exp::search(Exp* search, Exp*& result, bool typeSens /* = false */)
{
    std::list<Exp**> li;
    result = 0;             // In case it fails; don't leave it unassigned
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to
    // re-use the same search routine
    Exp* top = this;
    doSearch(search, typeSens, top, li, false);
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
 *                  results:  ref to list of Exp* that matched
 *                  typeSens: if true, comparisons are type sensitive
 * RETURNS:         True if a match was found
 *============================================================================*/
bool Exp::searchAll(Exp* search, std::list<Exp*>& result, bool typeSens /* = false */)
{
    std::list<Exp**> li;
    result.clear();
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to
    // re-use the same search routine
    Exp* pSrc = this;
    doSearch(search, typeSens, pSrc, li, false);
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
    std::vector<int>& integers, bool negate)
{
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
 * FUNCTION:        Exp::simplifyArith
 * OVERVIEW:        This method simplifies an expression consisting of + and -
 *                  at the top level. For example,
 *                  (%sp + 100) - (%sp + 92) will be simplified to 8.
 * NOTE:            Any expression can be so simplified
 * NOTE:            User must delete result
 * PARAMETERS:      <none>
 * RETURNS:         Ptr to the simplified expression
 *============================================================================*/
Exp* Exp::simplifyArith()
{
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
    int sum = accumulate(integers.begin(),integers.end(),0);

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
 *============================================================================*/
Exp* Exp::simplify() {
    bool bMod;                  // True if simplified at this or lower level
    Exp* res = this;
    do {
        bMod = false;
        res = res->polySimplify(bMod);// Call the polymorphic simplify
    } while (bMod);             // If modified at this (or a lower) level, redo
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
    switch (op) {
        case opNeg: case opNot: case opLNot: case opSize:
        {
            OPER subOP = subExp1->getOper();
            if (subOP == opIntConst) {
                // -k, ~k, or ~k
                res = ((Unary*)res)->becomeSubExp1();
                int k = ((Const*)res)->getInt();
                switch (op) {
                    case opNeg: k = -k; break; 
                    case opNot: k = ~k; break;
                    case opLNot:k = !k; break;
                    case opSize: /* No change required */ break;
                    default: break;
                }
                ((Const*)res)->setInt(k);
                bMod = true; 
            }
        }
        default: break;
    }
    return res;
}

Exp* Binary::polySimplify(bool& bMod) {
    Exp* res = this;
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

    // Might want to commute to put an integer constant on the RHS
    // Later simplifications can rely on this
    if (opSub1 == opIntConst) {
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

    // Check for exp * 1  or exp AND 1
    if ((op == opMult || op == opMults || (op == opAnd)) &&
      opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 1) {
        res = ((Unary*)res)->becomeSubExp1();
        bMod = true;
        return res;
    }

    // Next change: if we have x + k where k is a negative
    // integer constant, change this to x - p where p = -k 
    if (op == opPlus &&
      opSub2 == opIntConst && ((k = ((Const*)subExp2)->getInt()) < 0)) {
        res->setOper(opMinus);
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

    return res;
}

Exp* TypedExp::polySimplify(bool& bMod) {
    // This is likely to cause problems!
    Unary* p = (Unary*)this;
    return p->polySimplify(bMod);
}

/*==============================================================================
 * FUNCTION:        Exp::printt
 * OVERVIEW:        Print an infix representation of the object to the given
 *                  file stream, with it's type in <angle brackets>.
 * PARAMETERS:      Output stream to send the output to
 * RETURNS:         <nothing>
 *============================================================================*/
void Exp::printt(std::ostream& os /*= cout*/)
{
    print(os);
    if (op != opTypedExp) return;
    Type* t = &((TypedExp*)this)->getType();
    os << "<" << std::dec << t->getSize();
    switch (t->getType()) {
        case INTEGER:
            if (t->getSigned())
                        os << "i";              // Integer
            else
                        os << "u"; break;       // Unsigned
        case FLOATP:    os << "f"; break;
        case DATA_ADDRESS: os << "pd"; break;   // Pointer to Data
        case FUNC_ADDRESS: os << "pc"; break;   // Pointer to Code
        case VARARGS:   os << "v"; break;
        case BOOLEAN:   os << "b"; break;
        case UNKNOWN:   os << "?"; break;
        case TVOID:     break;
    }
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
    p->printt(os);
#else
    p->print(os);
#endif
    return os;
}

/*==============================================================================
 * FUNCTION:         Exp::decideType
 * OVERVIEW:         Scan this Exp; if its top level
 *                      operator std::decrees a type, then set ty to this type,
 *                      and return true
 * NOTE:             This version only inspects one expression level
 * NOTE:             Caller must delete ty if returns true
 * PARAMETERS:       assignSize - size of the assignment
 *                   ty: ref to ptr to Type as std::decided (only if return true)
 * RETURNS:          True if type is std::decided; ty updated if true
 *============================================================================*/
bool Exp::decideType(int assignSize, Type*& ty)
{
    int toSize;
    Exp* e;
    switch (op) {
        // Operators with explicit sizes
        case opFsize:
        case opItof:
            e = ((Ternary*)this)->getSubExp2();
            assert(e->getOper() == opIntConst);
            toSize = ((Const*)e)->getInt();
            ty = new Type(FLOATP, toSize, true);
            return true;

        case opFtoi:
        case opTruncs:
        case opSgnEx:
            e = ((Ternary*)this)->getSubExp2();
            assert(e->getOper() == opIntConst);
            toSize = ((Const*)e)->getInt();
            ty = new Type(INTEGER, toSize, true);
            return true;

        case opZfill:
        case opTruncu:
            e = ((Ternary*)this)->getSubExp2();
            assert(e->getOper() == opIntConst);
            toSize = ((Const*)e)->getInt();
            ty = new Type(INTEGER, toSize, false);
            return true;

        case opSize: {
            // NOTE: Opsize is deprecated
            // Beware... even though we know the size, we may not know the
            // broad type (e.g. m[]{size})
            e = ((Binary*)this)->getSubExp1();
            assert(e->getOper() == opIntConst);
            toSize = ((Const*)e)->getInt();
            Exp* sub = ((Binary*)this)->getSubExp2();
            if (sub->decideType(assignSize, ty)) {
                ty->setSize(toSize);
                return true;
            }
            else
                // We have a size but no type
                return false;
        }

        case opMult:            // Implicitly unsigned operators
        case opDiv:
        case opMod:
        case opShiftR:
            ty = new Type(INTEGER, assignSize, false);
            return true;

        case opMults:           // Implicitly signed operators
        case opDivs:
        case opMods:
        case opShiftRA:
        case opNeg:
        case opSignExt:
            ty = new Type(INTEGER, assignSize, true);
            return true;

        // Problem with these is std::deciding the size. In some cases, the
        // assign size seems to be wrong.
        // An example would be helpful
        case opFPlus:
        case opFMinus:
        case opFMult:
        case opFDiv:
        // MVE: idFplusd etc should no longer be used, but I believe that they
        // still are
        case opFPlusd:  case opFMinusd: case opFMultd:  case opFDivd:
        case opFPlusq:  case opFMinusq: case opFMultq:  case opFDivq:
        {
            Exp* sub = ((Binary*)this)->getSubExp2();
            if (sub->decideType(assignSize, ty)) {
                return true;
            }
        }
        // Fall through to the unary case
        case opSQRTs: case opSQRTd: case opSQRTq: case opSqrt:
        case opSin: case opCos: case opTan: case opArcTan:
        case opLog2: case opLog10: case opLoge: {
            Exp* sub = ((Unary*)this)->getSubExp1();
            if (sub->decideType(assignSize, ty)) {
                return true;
            }
            /* If we didn't get anything specific, just set the basic type
             * and assume the assign size is correct */
            ty = new Type(FLOATP, assignSize);
            return true;
        }

        case opFMultdq:
            ty = new Type(FLOATP, 128);
            return true;
        case opFMultsd:
            ty = new Type(FLOATP, 64);
            return true;

        case opTemp: {
            // Different to UQBT. In UQBT, it was r[ t[ j]], where j was a
            // special index into the semantic table. May have been some
            // without the r[].
            // Here, opTemp is a Unary, with a string constant subexpression
            e = ((Ternary*)this)->getSubExp1();
            assert(e->getOper() == opStrConst);
            char* p = ((Const*)e)->getStr();
            ty = Type::getTempType(p);
            return true;
        }
        case opRegOf: {
            e = ((Ternary*)this)->getSubExp1();
            if (e->getOper() == opIntConst) {
                int regNum = ((Const*)e)->getInt();
                // Get the register's intrinsic type
                // Ugh - using the global prog here...
                ty = new Type(prog.RTLDict.DetRegMap[regNum].g_type());
                return true;
            } else { /* r[param] or r[expr] */
                /* This is really only useful when processing uninstantiated
                 * rtls, and assumes someone has actually set the types in
                 * DetParamMap (the information isn't in the SSL)
                 */
                Unary wildParam(opParam, new Terminal(opWild));
                Exp* result;
                if (e->search(&wildParam, result)) {
                    Exp* strexp = ((Unary*)result)->getSubExp1();
                    char* p = ((Const*)strexp)->getStr();
                    std::map<std::string, ParamEntry>::iterator it;
                    // That global again (twice)...
                    it = prog.RTLDict.DetParamMap.find(p);
                    if (it != prog.RTLDict.DetParamMap.end()) {
                        ParamEntry& ent = it->second;
                        if (ent.regType.getType() != TVOID) {
                            ty = new Type(ent.regType.getType());
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        case opFltConst:
            // Deal with this in partialType, unless other side has better
            // type info
            return false;

        case opMemOf:
            // This operator has no relationship to its subexpressions, so we
            // have no type at all
            return false;

        case opNil:
            // Might happen e.g. RHS of FPOP
            return false;

        // Can't be a var; these are created later. Besides, we don't have a
        // reference to the needed proc object here
        default:
            // This is pretty weak. Usint the type of subexpressions works
            // often, but there are exceptions, e.g. shift operators.
            int a = getArity();
            if (a < 1) return false;
            e = ((Unary*)this)->getSubExp1();
            if (e && e->decideType(assignSize, ty)) return true;
            if (a < 2) return false;
            e = ((Binary*)this)->getSubExp2();
            if (e && e->decideType(assignSize, ty)) return true;
            if (a < 3) return false;
            e = ((Ternary*)this)->getSubExp3();
            if (e && e->decideType(assignSize, ty)) return true;
    }
    return false;               // Can't std::decide the type
}

