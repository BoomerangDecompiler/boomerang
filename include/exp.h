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
#include "dataflow.h"   // AssignExp derived from Statement

class UseSet;
class DefSet;
class RTL;              // For class FlagDef
class BasicBlock;	    // For class AssignExp
typedef BasicBlock* PBB;

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
virtual void print(std::ostream& os, bool withUses = false) = 0;
             // Print with <type>
    void     printt(std::ostream& os = std::cout, bool withUses = false);
    void     printAsHL(std::ostream& os = std::cout); // Print with v[5] as v5
    char*    prints();      // Print to string (for debugging)
             // Recursive print: don't want parens at the top level
virtual void printr(std::ostream& os, bool withUses = false) {
                print(os, withUses);}       // But most classes want standard
             // Print with the "{1 2 3}" uses info
        void printWithUses(std::ostream& os) {print(os, true);}

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
// Comparison ignoring subscripts
        bool operator*=(const Exp& o) const;

// Return the number of subexpressions. This is only needed in rare cases,
// Could use polymorphism for all those cases, but this is easier
virtual int getArity() {return 0;}      // Overridden for Unary, Binary, etc

    //  //  //  //  //  //  //
    //   Enquiry functions  //
    //  //  //  //  //  //  //

    // True if this is a call to a flag function
    bool isFlagCall() {return op == opFlagCall;}
    // True if this is an assignment to the "flags" abstract location
    bool isFlagAssgn();
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
    // True if this is a temporary. Note some old code still has r[tmp]
    bool isTemp();
    // True if this is the anull Terminal (anulls next instruction)
    bool isAnull() {return op == opAnull;}
    // True if this is the Nil Terminal (terminates lists; "NOP" expression)
    bool isNil() {return op == opNil;}
    // Trye if this is %pc
    bool isPC() {return op == opPC;}
    // True if is %afp, %afp+k, %afp-k, or a[m[<any of these]]
    bool isAfpTerm();
    // True if is int const
    bool isIntConst() {return op == opIntConst;}
    // True if is string const
    bool isStrConst() {return op == opStrConst;}
    // True if is flt point const
    bool isFltConst() {return op == opFltConst;}
    // True if is a post-var expression (var_op' in SSL file)
    bool isPostVar() {return op == opPostVar;}
    // True if this is an opSize (size case; deprecated)
    bool isSizeCast() {return op == opSize;}
    // True if this is a subscripted expression (SSA)
    bool isSubscript() {return op == opSubscript;}
    // True if this is a phi operation (SSA)
    bool isPhi() {return op == opPhi;}
    // Get the index for this var
    int getVarIndex();
    // True if this is a terminal
    virtual bool isTerminal() { return false; }
    // True if this is a comparison
    bool isComparison() { return op == opEquals || op == opNotEqual ||
                                 op == opGtr || op == opLess ||
                                 op == opGtrUns || op == opLessUns ||
                                 op == opGtrEq || op == opLessEq ||
                                 op == opGtrEqUns || op == opLessEqUns; }
           
                 

    //  //  //  //  //  //  //
    //  Search and Replace  //
    //  //  //  //  //  //  //
    
    // Search for Exp *search in this Exp. If found, return true and return
    // a ref to the matching expression in result (useful with wildcards).
    virtual bool search(Exp* search, Exp*& result);

    // Search for Exp search in this Exp. For each found, add
    // a ptr to the matching expression in result (useful with wildcards).    
    bool    searchAll(Exp* search, std::list<Exp*>& result);

    // Search this Exp for *search; if found, replace with *replace
    Exp* searchReplace (Exp* search, Exp* replace, bool& change);

    // Search *pSrc for *search; for all occurrences, replace with *replace
    Exp* searchReplaceAll(Exp* search, Exp* replace, bool& change,
        bool once = false);

    // Not for public use. Search for subexpression matches.
        void doSearch(Exp* search, Exp*& pSrc, std::list<Exp**>& li, bool once);

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
virtual void  setSubExp4(Exp* e) {};

    //  //  //  //  //  //  //
    //  Guarded assignment  //
    //  //  //  //  //  //  //
    Exp*    getGuard();         // Get the guard expression, or 0 if not
    
    //  //  //  //  //  //  //  //  //
    //  Expression Simplification   //
    //  //  //  //  //  //  //  //  //
    
    void    partitionTerms(std::list<Exp*>& positives, std::list<Exp*>& negatives,
        std::vector<int>& integers, bool negate);
virtual Exp*    simplifyArith() {return this;}
static Exp* Accumulate(std::list<Exp*> exprs);
    // Simplify the expression
    Exp*    simplify();
virtual Exp* polySimplify(bool& bMod) {bMod = false; return this;}
    // Just the address simplification a[ m[ any ]]
virtual Exp* simplifyAddr() {return this;}
virtual Exp* fixSuccessor() {return this;}
		// Kill any zero fill, sign extend, or truncates
		Exp* killFill();

    // Do the work of finding used locations
    virtual void addUsedLocs(LocationSet& used) {}

    // Update the "uses" information implicit in expressions
    // defs is a StatementSet of definitions reaching this statement
    virtual Exp* updateUses(StatementSet& defs) {return this;}
    // Add a subscript to this; may return a new Exp
    virtual Exp* addSubscript(Statement* def);

    // Get number of definitions (statements this expression depends on)
    virtual int getNumUses() {return 0;}

    // Convert from SSA form
    virtual Exp* fromSSA(igraph& ig) {return this;}

    // Consistency check. Might be useful another day
    void check();

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
            Const(ADDRESS a);
            Const(double d);
            Const(char* p);
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

    void    print(std::ostream& os, bool withUses = false);
    void    printNoQuotes(std::ostream& os, bool withUses = false);
    // Print "recursive" (extra parens not wanted at outer levels)

    void    appendDotFile(std::ofstream& of);

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

    // Nothing to destruct: Don't deallocate the string passed to constructor
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

    void    print(std::ostream& os, bool withUses = false);
    void    appendDotFile(std::ofstream& of);

    Exp*    updateUses(StatementSet& defs);
    
    // Do the work of finding used locations
    virtual void addUsedLocs(LocationSet& used);

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
    void    print(std::ostream& os, bool withUses = false);
    void    appendDotFile(std::ofstream& of);

    // Set first subexpression
    void    setSubExp1(Exp* e);
    void    setSubExp1ND(Exp* e) {subExp1 = e;}
    // Get first subexpression
    Exp*    getSubExp1();
    // "Become" subexpression 1 (delete all but that subexpression)
    Exp*    becomeSubExp1();
    // Get a reference to subexpression 1
    Exp*&   refSubExp1();
virtual Exp* fixSuccessor();

    // Search children
    void doSearchChildren(Exp* search, std::list<Exp**>& li, bool once);

    // Do the work of simplifying this expression
    Exp* polySimplify(bool& bMod);
    Exp* simplifyArith();
    Exp* simplifyAddr();

    // Do the work of finding used locations
    virtual void addUsedLocs(LocationSet& used);

    // Update the "uses" information implicit in expressions
    virtual Exp* updateUses(StatementSet& defs);

    // Convert from SSA form
    virtual Exp* fromSSA(igraph& ig);

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
    void    print(std::ostream& os, bool withUses = false);
    void    printr(std::ostream& os, bool withUses = false);
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
    void doSearchChildren(Exp* search, std::list<Exp**>& li, bool once);

    // Do the work of simplifying this expression
    Exp* polySimplify(bool& bMod);
    Exp* simplifyArith();
    Exp* simplifyAddr();

    // Do the work of finding used locations
    virtual void addUsedLocs(LocationSet& used);

    // Update the "uses" information implicit in expressions
    virtual Exp* updateUses(StatementSet& defs);

    // Convert from SSA form
    virtual Exp* fromSSA(igraph& ig);

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
    void    print(std::ostream& os, bool withUses = false);
    void    printr(std::ostream& os, bool withUses = false);
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
    void doSearchChildren(Exp* search, std::list<Exp**>& li, bool once);

    Exp* polySimplify(bool& bMod);
    Exp* simplifyArith();
    Exp* simplifyAddr();

    // Do the work of finding used locations
    virtual void addUsedLocs(LocationSet& used);

    // Update the "uses" information implicit in expressions
    // def is a statement defining left (pass left == getLeft(def))
    virtual Exp* updateUses(StatementSet& defs);

    // Convert from SSA form
    virtual Exp* fromSSA(igraph& ig);

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


    void    print(std::ostream& os, bool withUses = false);
    void    appendDotFile(std::ofstream& of);

    // Get and set the type
    Type*   getType();
    void    setType(Type* ty);

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

    virtual void print(std::ostream& os, bool withUses = false);
    void    appendDotFile(std::ofstream& of);

    // Get and set the size
    int	    getSize();
    void    setSize(int sz);

    // Do the work of simplifying this expression
    Exp* polySimplify(bool& bMod);
    Exp* simplifyArith();
    Exp* simplifyAddr();

	// serialization
	virtual bool serialize(std::ostream &ouf, int &len);

	// new dataflow analysis
    virtual void killDef(StatementSet &reach);
    virtual void killLive(LocationSet &live);
    virtual void killDead(LocationSet &dead);
    virtual void getDeadStatements(StatementSet &dead);
	virtual bool usesExp(Exp *e);
    virtual void addUsedLocs(LocationSet& used);
    // Update the "uses" information implicit in expressions
    // def is a statement defining left (pass left == getLeft(def))
    virtual Exp* updateUses(StatementSet& defs);

    virtual bool isDefinition() { return true; }
    virtual void getDefinitions(LocationSet &defs);
        
        // get how to access this value
    virtual Exp* getLeft() { return subExp1; }
	virtual Type* getLeftType() { return NULL; }

        // get how to replace this statement in a use
        virtual Exp* getRight() { return subExp2; }

	// special print functions
    //virtual void printAsUse(std::ostream &os);
    //virtual void printAsUseBy(std::ostream &os);

	// inline any constants in the statement
	virtual void processConstants(Prog *prog);

    // general search
    virtual bool search(Exp* search, Exp*& result) {
        return Exp::search(search, result);
    }

	// general search and replace
	virtual void searchAndReplace(Exp *search, Exp *replace) {
	    bool change;
	    Exp *e = searchReplaceAll(search, replace, change);
	    assert(e == this);
	}
 
    // update type for expression
    virtual Type *updateType(Exp *e, Type *curType);

    // to/from SSA form
    virtual void   toSSAform(StatementSet& reachin) {updateUses(reachin);}
    virtual void fromSSAform(igraph& ig);

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

/*==============================================================================
 * UsesExp is a subclass of Unary, holding an ordinary Exp pointer, and
 *  a StatementSet
 * This is used for subscripting SSA variables. Example:
 * m[1000] becomes m[1000]{3} if defined at statement 3
 * m[r[28]+4] becomes m[r[28]{2 8}]{3} if r[28] is defined at 2 and 8, and
 * the memof is defined at 3. The integers are really pointers to statements,
 * printed as the statement number for compactness
 *============================================================================*/
class UsesExp : public Unary {
    StatementSet    stmtSet;            // A set of pointers to statements

public:
            // Constructor with expression (e) and statement defining it (def)
            UsesExp(Exp* e, Statement* def);
            UsesExp(Exp* e);
            UsesExp(UsesExp& o);
virtual Exp* clone();
    bool    operator==(const Exp& o) const;
    void    print(std::ostream& os, bool withUses = false);
    Exp*    updateUses(StatementSet& defs);
virtual int getNumUses() {return stmtSet.size();}
    Statement* getFirstUses() {StmtSetIter it; return stmtSet.getFirst(it);}
    void    addUsedLocs(LocationSet& used);
virtual Exp* addSubscript(Statement* def) {
                stmtSet.insert(def); return this;}
    Statement* getFirstUse(StmtSetIter it) {return stmtSet.getFirst(it);}
    Statement* getNextUse (StmtSetIter it) {return stmtSet.getNext (it);}
    virtual Exp* fromSSA(igraph& ig);
};

    
#endif // __EXP_H__
