/*==============================================================================
 * FILE:       exp.h
 * OVERVIEW:   Provides the definition for the Exp class and its
 *             subclasses.
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 */

#ifndef __EXP_H_
#define __EXP_H_

#include <fstream>      // For ostream, cout etc
#include "index.h"      // Declares the INDEX enum
#include "types.h"      // For ADDRESS, etc
#include <stdio.h>      // For sprintf

/*============================================================================== * Exp is an expression class, though it will probably be used to hold many
 * other things (e.g. perhaps transformations). It is a standard tree
 * representation. Exp itself is abstract. A special subclass Const is used
 * for constants. Unary, Binary, and Ternary hold 1, 2, and 3 subexpressions
 * respectively. For efficiency of representation, these have to be separate
 * classes, derived from Exp.
 *============================================================================*/

// Class Exp is abstract, in the sense that no instance of this class should
// ever be instantiated. However, the constructor can be called from the
// constructors of derived classes
class Exp {
protected:
    INDEX   id;             // The operator ID (e.g. idPlus)

protected:
    // Constructor, with ID
            Exp(INDEX id) : id(id) {};
    // Print the expression to the given stream
public:
    void    print(ostream& os = cout);
};

/*==============================================================================
 * Const is a subclass of Exp, and holds either an integer, floating point,
 * string, or address constant
 *============================================================================*/
class Const : public Exp {
    union {
        int i;          // Integer
        double d;       // Double precision float
        const char* p;  // Pointer to string
        ADDRESS a;      // Code address
    } u;
public:
    // Special constructors overloaded for the various constants
            Const(int i);
            Const(double d);
            Const(const char* p);
            Const(ADDRESS a);

    void    print(ostream& os);
    // Nothing to destruct: Don't deallocate the string passed to constructor
};

/*==============================================================================
 * Unary is a subclass of Exp, holding one subexpression
 *============================================================================*/
class Unary : public Exp {
    Exp*        subExp1;    // One subexpression pointer
public:
    // Constructor, with just ID
            Unary(INDEX id);
    // Constructor, with ID and subexpression
            Unary(INDEX id, Exp* e);
    // Destructor
            ~Unary();
    // Set first subexpression
    void    setSubExp1(Exp* e);
    // Get first subexpression
    Exp*    getSubExp1();
};

/*==============================================================================
 * Binary is a subclass of Unary, holding two subexpressions
 *============================================================================*/
class Binary : public Unary {
    Exp*        subExp2;    // Second subexpression pointer
public:
    // Constructor, with ID
            Binary(INDEX id);
    // Constructor, with ID and subexpressions
            Binary(INDEX id, Exp* e1, Exp* e2);
    // Destructor
            ~Binary();
    // Set second subexpression
    void    setSubExp2(Exp* e);
    // Get second subexpression
    Exp*    getSubExp2();
};

/*==============================================================================
 * Ternary is a subclass of Binary, holding three subexpressions
 *============================================================================*/
class Ternary : public Binary {
    Exp*        subExp3;    // Third subexpression pointer
public:
    // Constructor, with ID
            Ternary(INDEX id);
    // Constructor, with ID and subexpressions
            Ternary(INDEX id, Exp* e1, Exp* e2, Exp* e3);
    // Destructor
            ~Ternary();
    // Set third subexpression
    void    setSubExp3(Exp* e);
    // Get third subexpression
    Exp*    getSubExp3();
};



#endif // __EXP_H__
