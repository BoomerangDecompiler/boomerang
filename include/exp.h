/*
 * Copyright (C) 2002, Trent Waddington
 */
/*==============================================================================
 * FILE:       exp.h
 * OVERVIEW:   Provides the definition for the Exp class and its
 *             subclasses.
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 * 05 Apr 02 - Mike: Added clone(), copy constructors
 * 08 Apr 02 - Mike: Added isFlagCall(), Terminal subclass
 * 10 Apr 02 - Mike: Search and replace
 * 29 Apr 02 - Mike: TypedExp takes Type& and Exp* in opposite order; consistent
 * 10 May 02 - Mike: Added refSubExp1 etc
 * 21 May 02 - Mike: Mods for gcc 3.1
 */

#ifndef __EXP_H_
#define __EXP_H_

#include <iostream>
#include <fstream>      // For ostream, cout etc
#include <stdio.h>      // For sprintf
#include <list>
#include <vector>
#include <set>
#include "operator.h"   // Declares the OPER enum
#include "types.h"      // For ADDRESS, etc
#include "type.h"       // The Type class for typed expressions

class UseSet;
class DefSet;
class RTL;              // For class FlagDef
class BasicBlock;	// For class AssignExp
typedef BasicBlock* PBB;
class Statement;

/*==============================================================================
 * Exp is an expression class, though it will probably be used to hold many
 * other things (e.g. perhaps transformations). It is a standard tree
 * representation. Exp itself is abstract. A special subclass Const is used
 * for constants. Unary, Binary, and Ternary hold 1, 2, and 3 subexpressions
 * respectively. For efficiency of representation, these have to be separate
 * classes, derived from Exp.
 *============================================================================*/

// Class Exp is abstract. However, the constructor can be called from the
// constructors of derived classes
class Exp {
protected:
    OPER   op;             // The operator (e.g. opPlus)

    // Constructor, with ID
            Exp(OPER op) : op(op) {}
public:
	// Virtual destructor
virtual		~Exp() {}

    // Return the index. Note: I'd like to make this protected, but then
    // subclasses don't seem to be able to use it (at least, for subexpressions)
    OPER    getOper() const {return op;}
    void    setOper(OPER x) {op = x;}     // A few simplifications use this

    // Print the expression to the given stream
	// Don't default to std::cout because it makes debugging harder
virtual void print(std::ostream& os) = 0;
	void	 print() {print(std::cout);}
    void     printt(std::ostream& os = std::cout);    // Print with <type>
    void     printAsHL(std::ostream& os = std::cout); // Print with v[5] as v5
    char*    prints();      // Print to string (for debugging)
             // Recursive print: don't want parens at the top level
virtual void printr(std::ostream& os) = 0;    // Recursive print

    // Display as a dotty graph
    void    createDotFile(char* name);
virtual void appendDotFile(std::ofstream& os) = 0;

    // Clone (make copy of self that can be deleted without affecting self)
virtual Exp* clone() = 0;

    // Comparison
// Type sensitive equality
virtual bool operator==(const Exp& o) const = 0;
// NOTE: All these type insensitive comparisons may go away because only
// assignments, parameters, and return locations will be typed
// They are still here in case they are still needed, and because it makes
// the port from UQBT code easier
// Type insensitive equality
virtual bool operator%=(const Exp& o) const;
// Sign insensitive equality
virtual bool operator-=(const Exp& o) const;
// Type sensitive less than
virtual bool operator< (const Exp& o)  const= 0;
// Type insensitive less than. Class TypedExp overrides
virtual bool operator<<(const Exp& o) const
    {return (*this < o);}

// Return the number of subexpressions. This is only needed in rare cases,
// Could use polymorphism for all those cases, but this is easier
virtual int getArity() {return 0;}      // Overridden for Unary, Binary, etc

    //  //  //  //  //  //  //
    //   Enquiry functions  //
    //  //  //  //  //  //  //

    // True if this is an assignment to the abstract flags register
    bool isFlagCall() {return op == opFlagCall;}
    // True if this is an ordinary (non flags) assignment
    bool isAssign();
    // True if this is a register location
    bool isRegOf() {return op == opRegOf;}
    // True if this is a register location with a constant index
    bool isRegOfK();
    // True if this is a specific numeric register
    bool isRegN(int n);
    // True if this is a memory location
    bool isMemOf() {return op == opMemOf;}
    // True if this is an address of
    bool isAddrOf() {return op == opAddrOf;}
    // True if this is a temporary
    bool isTemp() {return op == opTemp;}
    // True if this is the anull Terminal (anulls next instruction)
    bool isAnull() {return op == opAnull;}
    // True if this is the Nil Terminal (terminates lists; "NOP" expression)
    bool isNil() {return op == opNil;}
    // True if is %afp, %afp+k, %afp-k, or a[m[<any of these]]
    bool isAfpTerm();
    // True if is int const
    bool isIntConst() {return op == opIntConst;}
    // True if is addr const
    bool isAddrConst() {return op == opAddrConst;}
    // True if is flt point const
    bool isFltConst() {return op == opFltConst;}
    // True if is a post-var expression (var_op' in SSL file)
    bool isPostVar() {return op == opPostVar;}
    // True if this is an opSize (size case; deprecated)
    bool isSizeCast() { return op == opSize;}
    // Get the index for this var
    int getVarIndex();
    // True if this is a terminal
    virtual bool isTerminal() { return false; }

    //  //  //  //  //  //  //
    //  Search and Replace  //
    //  //  //  //  //  //  //
    
    // Search for Exp *search in this Exp. If found, return true and return
    // a ref to the matching expression in result (useful with wildcards).
    bool    search(Exp* search, Exp*& result);

    // Search for Exp search in this Exp. For each found, add
    // a ptr to the matching expression in result (useful with wildcards).    
    bool    searchAll(Exp* search, std::list<Exp*>& result);

    // Search this Exp for *search; if found, replace with *replace
    Exp* searchReplace (Exp* search, Exp* replace, bool& change);

    // Search *pSrc for *search; for all occurrences, replace with *replace
    Exp* searchReplaceAll(Exp* search, Exp* replace, bool& change,
        bool once = false);

    // Not for public use. Search for subexpression matches.
static void doSearch(Exp* search, Exp*& pSrc, std::list<Exp**>& li,
        bool once);

    // As above.
virtual void doSearchChildren(Exp* search, std::list<Exp**>& li,
          bool once);

    //  //  //  //  //  //  //
    //    Sub expressions   //
    //  //  //  //  //  //  //
    
    // These are here so we can (optionally) prevent code clutter.
    // Using a *Exp (that is known to be a Binary* say), you can just
    // directly call getSubExp2.
    // However, you can still choose to cast from Exp* to Binary* etc.
    // and avoid the virtual call
virtual Exp*  getSubExp1() {return 0;}
virtual Exp*  getSubExp2() {return 0;}
virtual Exp*  getSubExp3() {return 0;}
virtual Exp*& refSubExp1();
virtual Exp*& refSubExp2();
virtual Exp*& refSubExp3();
virtual void  setSubExp1(Exp* e) {};
virtual void  setSubExp2(Exp* e) {};
virtual void  setSubExp3(Exp* e) {};

    //  //  //  //  //  //  //
    //  Guarded assignment  //
    //  //  //  //  //  //  //
    Exp*    getGuard();         // Get the guard expression, or 0 if not
    
    //  //  //  //  //  //  //  //  //
    //  Expression Simplification   //
    //  //  //  //  //  //  //  //  //
    
    void    partitionTerms(std::list<Exp*>& positives, std::list<Exp*>& negatives,
        std::vector<int>& integers, bool negate);
    Exp*    simplifyArith();
static Exp* Accumulate(std::list<Exp*> exprs);
    // Simplify the expression
    Exp*    simplify();
virtual Exp* polySimplify(bool& bMod) {bMod = false; return this;}
    // Just the address simplification a[ m[ any ]]
virtual Exp* simplifyAddr() {return this;}

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len) = 0;
	static Exp *deserialize(std::istream &inf);

};

// Not part of the Exp class, but logically belongs with it:
std::ostream& operator<<(std::ostream& os, Exp* p);  // Print the Exp poited to by p

/*==============================================================================
 * Const is a subclass of Exp, and holds either an integer, floating point,
 * string, or address constant
 *============================================================================*/
class Const : public Exp {
    union {
        int i;          // Integer
        double d;       // Double precision float
        char* p;        // Pointer to string
        ADDRESS a;      // Code address
    } u;
public:
    // Special constructors overloaded for the various constants
            Const(int i);
            Const(double d);
            Const(char* p);
            Const(ADDRESS a);
    // Copy constructor
            Const(Const& o);
            
    // Clone
    virtual Exp* clone();

    // Compare
    bool    operator==(const Exp& o) const;
    bool    operator< (const Exp& o) const;

    // Get the constant
    int     getInt() {return u.i;}
    double  getFlt() {return u.d;}
    char*   getStr() {return u.p;}
    ADDRESS getAddr(){return u.a;}

    // Set the constant
    void setInt(int i)      {u.i = i;}
    void setFlt(double d)   {u.d = d;}
    void setStr(char* p)    {u.p = p;}
    void setAddr(ADDRESS a) {u.a = a;}

    void    print(std::ostream& os);
    void    printNoQuotes(std::ostream& os);
    void    printr(std::ostream& os) {print (os);}
    // Nothing to destruct: Don't deallocate the string passed to constructor

    void    appendDotFile(std::ofstream& of);

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

};

/*==============================================================================
 * Terminal is a subclass of Exp, and holds special zero arity items such
 * as opFlags (abstract flags register)
 *============================================================================*/
class Terminal : public Exp {
public:
    // Constructors
        Terminal(OPER op);
        Terminal(Terminal& o);      // Copy constructor

    // Clone
    virtual Exp* clone();

    // Compare
    bool    operator==(const Exp& o) const;
    bool    operator< (const Exp& o) const;

    void    print(std::ostream& os);
    void    printr(std::ostream& os) {print (os);}
    void    appendDotFile(std::ofstream& of);

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

    virtual bool isTerminal() { return true; }
};

/*==============================================================================
 * Unary is a subclass of Exp, holding one subexpression
 *============================================================================*/
class Unary : public Exp {
protected:
    Exp*        subExp1;    // One subexpression pointer
public:
    // Constructor, with just ID
            Unary(OPER op);
    // Constructor, with ID and subexpression
            Unary(OPER op, Exp* e);
    // Copy constructor
            Unary(Unary& o);

    // Clone
    virtual Exp* clone();

    // Compare
    bool    operator==(const Exp& o) const;
    bool    operator< (const Exp& o) const;

    // Destructor
virtual     ~Unary();

    // Arity
    int getArity() {return 1;}

    // Print
    void    print(std::ostream& os);
    // Only binary and higher arity get parentheses, so printr == print
    void    printr(std::ostream& os) {print (os);}
    void    appendDotFile(std::ofstream& of);

    // Set first subexpression
    void    setSubExp1(Exp* e);
    // Get first subexpression
    Exp*    getSubExp1();
    // "Become" subexpression 1 (delete all but that subexpression)
    Exp*    becomeSubExp1();
    // Get a reference to subexpression 1
    Exp*&   refSubExp1();

    // Search children
    void doSearchChildren(Exp* search,
      std::list<Exp**>& li, bool once);

    // Do the work of simplifying this expression
    Exp* polySimplify(bool& bMod);
    Exp* simplifyAddr();

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

};

/*==============================================================================
 * Binary is a subclass of Unary, holding two subexpressions
 *============================================================================*/
class Binary : public Unary {
protected:
    Exp*        subExp2;    // Second subexpression pointer
public:
    // Constructor, with ID
            Binary(OPER op);
    // Constructor, with ID and subexpressions
            Binary(OPER op, Exp* e1, Exp* e2);
    // Copy constructor
            Binary(Binary& o);

	// Clone
    virtual Exp* clone();

    // Compare
    bool    operator==(const Exp& o) const ;
    bool    operator< (const Exp& o) const ;

    // Destructor
virtual     ~Binary();

    // Arity
    int getArity() {return 2;}

    // Print
    void    print(std::ostream& os);
    void    printr(std::ostream& os);
    void    appendDotFile(std::ofstream& of);

    // Set second subexpression
    void    setSubExp2(Exp* e);
    // Get second subexpression
    Exp*    getSubExp2();
    // "Become" subexpression 2 (delete all but that subexpression)
    Exp*    becomeSubExp2();
    // Commute the two operands
    void    commute();
    // Get a reference to subexpression 2
    Exp*&   refSubExp2();

    // Search children
    void doSearchChildren(Exp* search, 
      std::list<Exp**>& li, bool once);

    // Do the work of simplifying this expression
    Exp* polySimplify(bool& bMod);
    Exp* simplifyAddr();

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

};

/*==============================================================================
 * Ternary is a subclass of Binary, holding three subexpressions
 *============================================================================*/
class Ternary : public Binary {
    Exp*        subExp3;    // Third subexpression pointer
public:
    // Constructor, with operator
            Ternary(OPER op);
    // Constructor, with operator and subexpressions
            Ternary(OPER op, Exp* e1, Exp* e2, Exp* e3);
    // Copy constructor
            Ternary(Ternary& o);

    // Clone
    virtual Exp* clone();

    // Compare
    bool    operator==(const Exp& o) const ;
    bool    operator< (const Exp& o) const ;

    // Destructor
virtual     ~Ternary();

    // Arity
    int getArity() {return 3;}

    // Print
    void    print(std::ostream& os);
    void    printr(std::ostream& os);
    void    appendDotFile(std::ofstream& of);

    // Set third subexpression
    void    setSubExp3(Exp* e);
    // Get third subexpression
    Exp*    getSubExp3();
    // "Become" subexpression 3 (delete all but that subexpression)
    Exp*    becomeSubExp3();
    // Get a reference to subexpression 3
    Exp*&   refSubExp3();

    // Search children
    void doSearchChildren(Exp* search, 
      std::list<Exp**>& li, bool once);

    Exp* polySimplify(bool& bMod);
    Exp* simplifyAddr();

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

};

/*==============================================================================
 * TypedExp is a subclass of Unary, holding one subexpression and a Type
 *============================================================================*/
class TypedExp : public Unary {
    Type   *type;
public:
    // Constructor
            TypedExp();
    // Constructor, subexpression
            TypedExp(Exp* e1);
    // Constructor, type, and subexpression.
    // A rare const parameter allows the common case of providing a temporary,
    // e.g. foo = new TypedExp(Type(INTEGER), ...);
            TypedExp(Type* ty, Exp* e1);
    // Copy constructor
            TypedExp(TypedExp& o);

    // Clone
    virtual Exp* clone();

    // Compare
    bool    operator==(const Exp& o) const;
    bool    operator%=(const Exp& o) const;        // Type insensitive compare
    bool    operator-=(const Exp& o) const;        // Sign insensitive compare
    bool    operator< (const Exp& o) const;
    bool    operator<<(const Exp& o) const;


    void    print(std::ostream& os);
    void    printr(std::ostream& os) {print(os);}     // Printr same as print
    void    appendDotFile(std::ofstream& of);

    // Get and set the type
    Type*   getType();
    void    setType(Type* ty);

    // Search children
    void doSearchChildren(Exp* search, 
      std::list<Exp**>& li, bool once);

    // Do the work of simplifying this expression
    Exp* polySimplify(bool& bMod);
    Exp* simplifyAddr();

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

};

/*==============================================================================
 * AssignExp is a subclass of Binary, holding two subexpressions and a size
 *============================================================================*/
class AssignExp : public Binary, public Statement {
    int size;
public:
    // Constructor
            AssignExp();
    // Constructor, subexpression
            AssignExp(Exp* lhs, Exp* rhs);
    // Constructor, size, and subexpressions.
            AssignExp(int sz, Exp* lhs, Exp* rhs);
    // Copy constructor
            AssignExp(AssignExp& o);

    // Clone
    virtual Exp* clone();

    // Compare
    bool    operator==(const Exp& o) const;
    bool    operator< (const Exp& o) const;

    virtual void print(std::ostream& os);
    void    printr(std::ostream& os) {print(os);}     // Printr same as print
    void    appendDotFile(std::ofstream& of);

    // Get and set the size
    int	    getSize();
    void    setSize(int sz);

    // Search children
    void doSearchChildren(Exp* search, 
      std::list<Exp**>& li, bool once);

    // Do the work of simplifying this expression
    Exp* polySimplify(bool& bMod);
    Exp* simplifyAddr();

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

	// new dataflow analysis
        virtual void killLive(std::set<Statement*> &live);
        virtual void getDeadStatements(std::set<Statement*> &dead);
	virtual bool usesExp(Exp *e);

        // get how to access this value
        virtual Exp* getLeft() { return subExp1; }

        // get how to replace this statement in a use
        virtual Exp* getRight() { return subExp2; }

	// dataflow print functions
        virtual void printWithLives(std::ostream& os);
        virtual void printWithUses(std::ostream& os);

	// special print functions
        virtual void printAsUse(std::ostream &os);
        virtual void printAsUseBy(std::ostream &os);

	// inline any constants in the statement
	virtual void inlineConstants(Prog *prog);

protected:
	virtual void doReplaceUse(Statement *use);
};

/*==============================================================================
 * FlagDef is a subclass of Unary, and holds a list of parameters (in the
 *  subexpression), and a pointer to an RTL
 *============================================================================*/
class FlagDef : public Unary {
    RTL*    rtl;
public:
            FlagDef(Exp* params, RTL* rtl);     // Constructor
virtual     ~FlagDef();                         // Destructor
    void    appendDotFile(std::ofstream& of);
	RTL*	getRtl() { return rtl; }
	void	setRtl(RTL* r) { rtl = r; }

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);
};

/*
 * A class for comparing Exp*s (comparing the actual expressions)
 * Type sensitive
 */
class lessExpStar : public std::binary_function<Exp*, Exp*, bool> {
public:
    bool operator()(const Exp* x, const Exp* y) const
        {
            return (*x < *y);       // Compare the actual Exps
        }
};

/*
 * A class for comparing Exp*s (comparing the actual expressions)
 * Type insensitive
 */
class lessTI : public std::binary_function<Exp*, Exp*, bool> {
public:
    bool operator()(const Exp* x, const Exp* y) const
        {
            return (*x << *y);      // Compare the actual Exps
        }
};



#endif // __EXP_H__
