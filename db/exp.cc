/*==============================================================================
 * FILE:       exp.cc
 * OVERVIEW:   Implementation of the Exp and related classes.
 *============================================================================*/
/*
 * $Revision$
 * 05 Apr 02 - Mike: Created
 */


#include "exp.h"

/*==============================================================================
 * FUNCTION:        Const::Const etc
 * OVERVIEW:        Constructors.
 * PARAMETERS:      As required
 * RETURNS:         <nothing>
 *============================================================================*/

// Derived class constructors

Const::Const(int i)        : Exp(idIntConst)   {u.i = i;}
Const::Const(double d)     : Exp(idFltConst)   {u.d = d;}
Const::Const(const char* p): Exp(idStrConst)   {u.p = p;}
Const::Const(ADDRESS a)    : Exp(idCodeAddr)   {u.a = a;}

Unary::Unary(INDEX id)
    : Exp(id)
{
    subExp1 = 0;        // Initialise the pointer
}
Unary::Unary(INDEX id, Exp* e)
    : Exp(id)
{
    subExp1 = e;        // Initialise the pointer
}
Binary::Binary(INDEX id)
    : Unary(id)
{
    subExp2 = 0;        // Initialise the 2nd pointer. The first
                        // pointer is initialised in the Unary constructor
}
Binary::Binary(INDEX id, Exp* e1, Exp* e2)
    : Unary(id, e1)
{
    subExp2 = e2;       // Initialise the 2nd pointer
}
Ternary::Ternary(INDEX id)
    : Binary(id)
{
    subExp3 = 0;
}
Ternary::Ternary(INDEX id, Exp* e1, Exp* e2, Exp* e3)
    : Binary(id, e1, e2)
{
    subExp3 = e3;
}

/*==============================================================================
 * FUNCTION:        Unary::~Unary etc
 * OVERVIEW:        Destructors.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
Unary::~Unary()
{
    // Remember to delete all children
    if (subExp1 != 0) delete subExp1;
}
Binary::~Binary()
{
    if (subExp2 != 0) delete subExp2;
    // Note that the first pointer is destructed in the Exp1 destructor
}
Ternary::~Ternary()
{
    if (subExp3 != 0) delete subExp3;
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
Exp* Unary::getSubExp1()
{
    return subExp1;
}
Exp* Binary::getSubExp2()
{
    return subExp2;
}
Exp* Ternary::getSubExp3()
{
    return subExp3;
}

/*==============================================================================
 * FUNCTION:        Exp::print
 * OVERVIEW:        "Print" in infix notation the expression to a stream
 *                  Mainly for debugging, or maybe some low level windows
 * PARAMETERS:      Ref to an output stream
 * RETURNS:         <nothing>
 *============================================================================*/
// Print the expression
void Exp::print(ostream& os)
{
    Exp* p1; Exp* p2; Exp* p3;
    switch (id) {

        //  //  //  //  //  //  //
        //   Binary operators   //
        //  //  //  //  //  //  //
        case idPlus:    case idMinus:
        case idMult:    case idMults:
        case idDiv:     case idDivs:
        case idMod:     case idMods:
        case idFPlus:   case idFMinus:
        case idFMult:   case idFDiv:
        case idAnd:     case idOr:
        case idEquals:  case idNotEqual:
        case idLess:    case idGtr:
        case idLessEq:  case idGtrEq:
        case idLessUns: case idGtrUns:
        case idLessEqUns: case idGtrEqUns:

            p1 = ((Binary*)this)->getSubExp1();
            p2 = ((Binary*)this)->getSubExp2();
            p1->print(os);
            switch (id) {
                case idPlus:    os << " + ";  break;
                case idMinus:   os << " - ";  break;
                case idMult:    os << " * ";  break;
                case idMults:   os << " *! "; break;
                case idDiv:     os << " / ";  break;
                case idDivs:    os << " /! "; break;
                case idMod:     os << " % ";  break;
                case idMods:    os << " %! "; break;
                case idFPlus:   os << " +f "; break;
                case idFMinus:  os << " -f "; break;
                case idFMult:   os << " *f "; break;
                case idFDiv:    os << " /f "; break;
                case idAnd:     os << " and ";break;
                case idOr:      os << " or "; break;
                case idEquals:  os << " = ";  break;
                case idNotEqual:os << " ~= "; break;
                case idLess:    os << " < ";  break;
                case idGtr:     os << " > ";  break;
                case idLessEq:  os << " <= "; break;
                case idGtrEq:   os << " >= "; break;
                case idLessUns: os << " <=u ";break;
                case idGtrUns:  os << " >u "; break;
                case idLessEqUns:os << " <=u ";break;
                case idGtrEqUns: os << " >=u ";break;

                default:    break;    // Not possible
            }
            p2->print(os);
            break;

        //  //  //  //  //  //  //
        //      Constants       //
        //  //  //  //  //  //  //
        case idIntConst:
        case idFltConst:
        case idStrConst:
        case idCodeAddr:
            ((Const*)this)->print(os);
            break;


        //  //  //  //  //  //  //
        //  x[ subexpression ]  //
        //  //  //  //  //  //  //
        case idRegOf:   case idMemOf:
        case idAddrOf:  case idVar:
        case idTemp:
            switch (id) {
                case idRegOf: os << "r["; break;
                case idMemOf: os << "m["; break;
                case idAddrOf:os << "a["; break;
                case idVar:   os << "v["; break;
                case idTemp:   os << "tmp["; break;
            }
            p1 = ((Unary*)this)->getSubExp1();
            p1->print(os);
            os << "]";
            break;

        //  //  //  //  //  //  //
        //    Unary operators   //
        //  //  //  //  //  //  //
        case idNot:     case idLNot:
        case idNeg:
            switch (id) {
                case idNot:     os << "~";  break;
                case idLNot:    os << "L~"; break;
                case idNeg:     os << "-"; break;
            }
            p1 = ((Unary*)this)->getSubExp1();
            p1->print(os);
            break;
        case idSgnEx:      // Different because the operator appears last
            p1 = ((Unary*)this)->getSubExp1();
            p1->print(os);
            os << "! ";

        //  //  //  //  //  //  //
        //  Ternary operators   //
        //  //  //  //  //  //  //
        case idTern:    case idAt:      // Ternary operators
            p1 = ((Ternary*)this)->getSubExp1();
            p2 = ((Ternary*)this)->getSubExp2();
            p3 = ((Ternary*)this)->getSubExp3();
            p1->print(os);
            if (id == idTern)
                os << "? ";
            else
                os << "@";
            p2->print(os);
            os << ":";
            p3->print(os);
            break;
        default:
            os << "!" << dec << id << "!";
    }
}

/*==============================================================================
 * FUNCTION:        Const::print
 * OVERVIEW:        Not intended to be called directly. The constants are
 *                  a slightly special case; this code keeps the union of
 *                  constant values a totally private member
 * PARAMETERS:      Ref to an output stream
 * RETURNS:         <nothing>
 *============================================================================*/
void Const::print(ostream& os)
{
    switch (id) {
        case idIntConst:
            os << dec << ((Const*)this)->u.i;
            break;
        case idFltConst:
            char buf[64];
            sprintf(buf, "%g", ((Const*)this)->u.d);
            os << buf;
            break;
        case idStrConst:
            os << """" << ((Const*)this)->u.p << """";
            break;
        case idCodeAddr: 
            os << "0x" << hex << ((Const*)this)->u.a;
            break;
    }
}
