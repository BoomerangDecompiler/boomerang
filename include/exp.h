/*
 * Copyright (C) 2002, Trent Waddington
 */
/*==============================================================================
 * FILE:	   exp.h
 * OVERVIEW:   Provides the definition for the Exp class and its
 *			   subclasses.
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
 * 02 Aug 04 - Mike: Removed PhiExp (PhiAssign replaces it) 
 * 05 Aug 04 - Mike: Removed the withUses/withDF parameter from print() funcs
 */

#ifndef __EXP_H_
#define __EXP_H_

/* Main class hierarchy:	Exp (abstract)
					  _____/ | \
					 /		 |	\
				  Unary	   Const Terminal
	 TypedExp____/	|	\		  \
	  FlagDef___/ Binary Location  TypeVal
	   RefExp__/	|
				 Ternary
*/

#include <iostream>
#include <fstream>		// For ostream, cout etc
#include <stdio.h>		// For sprintf
#include <list>
#include <vector>
#include <set>
#include <assert.h>
#include "operator.h"	// Declares the OPER enum
#include "types.h"		// For ADDRESS, etc
#include "type.h"		// The Type class for typed expressions
#include "statement.h"	// For StmtSet etc
#include "exphelp.h"
#include "memo.h"

class UseSet;
class DefSet;
class RTL;				// For class FlagDef
class Statement;
class BasicBlock;
class LocationSet;
class StatementSet;
class TypeVal;
class ExpVisitor;
class ExpModifier;
class XMLProgParser;
typedef BasicBlock* PBB;
typedef std::map<Exp*, int, lessExpStar> igraph;

/*==============================================================================
 * Exp is an expression class, though it will probably be used to hold many
 * other things (e.g. perhaps transformations). It is a standard tree
 * representation. Exp itself is abstract. A special subclass Const is used
 * for constants. Unary, Binary, and Ternary hold 1, 2, and 3 subexpressions
 * respectively. For efficiency of representation, these have to be separate
 * classes, derived from Exp.
 *============================================================================*/

// Class Exp is abstract. However, the constructor can be called from the 
// the constructors of derived classes, and virtual functions not overridden
// by derived classes can be called
class Exp : public Memoisable {
protected:
	OPER   op;			   // The operator (e.g. opPlus)

	unsigned int lexBegin, lexEnd;

	// Constructor, with ID
			Exp(OPER op) : op(op) {}

public:
	// Virtual destructor
virtual		~Exp() {}

	// Return the index. Note: I'd like to make this protected, but then
	// subclasses don't seem to be able to use it (at least, for subexpressions)
	OPER	getOper() const {return op;}
	void	setOper(OPER x) {op = x;}	  // A few simplifications use this

	void	setLexBegin(unsigned int n) { lexBegin = n; }
	void	setLexEnd(unsigned int n) { lexEnd = n; }
	unsigned int getLexBegin() { return lexBegin; }
	unsigned int getLexEnd() { return lexEnd; }

	// Print the expression to the given stream
virtual void print(std::ostream& os) = 0;
			 // Print with <type>
	void	printt(std::ostream& os = std::cout);
	void	printAsHL(std::ostream& os = std::cout); // Print with v[5] as v5
	char*	prints();	   // Print to string (for debugging)
			// Recursive print: don't want parens at the top level
virtual void printr(std::ostream& os) {
				 print(os);}	   // But most classes want standard
			 // For debugging: print in indented hex. In gdb: "p x->printx(0)"
virtual void printx(int ind) = 0;

	// Display as a dotty graph
	void	createDotFile(char* name);
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
virtual bool operator< (const Exp& o)  const = 0;
// Type insensitive less than. Class TypedExp overrides
virtual bool operator<<(const Exp& o) const
	{return (*this < o);}
// Comparison ignoring subscripts
virtual bool operator*=(Exp& o) = 0;

// Return the number of subexpressions. This is only needed in rare cases,
// Could use polymorphism for all those cases, but this is easier
virtual int getArity() {return 0;}		// Overridden for Unary, Binary, etc

	//	//	//	//	//	//	//
	//	 Enquiry functions	//
	//	//	//	//	//	//	//

	// True if this is a call to a flag function
	bool isFlagCall() {return op == opFlagCall;}
	// True if this represents one of the abstract flags locations, int or float
	bool isFlags() {return op == opFlags || op == opFflags;}
	// True if this is a register location
	bool isRegOf() {return op == opRegOf;}
	// True if this is a register location with a constant index
	bool isRegOfK();
	// True if this is a specific numeric register
	bool isRegN(int n);
	// True if this is a memory location (any memory nesting depth)
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
	// Get string constant even if mangled
	char* getAnyStrConst();
	// True if is flt point const
	bool isFltConst() {return op == opFltConst;}
	// True if is a post-var expression (var_op' in SSL file)
	bool isPostVar() {return op == opPostVar;}
	// True if this is an opSize (size case; deprecated)
	bool isSizeCast() {return op == opSize;}
	// True if this is a subscripted expression (SSA)
	bool isSubscript() {return op == opSubscript;}
	// True if this is a phi assignmnet (SSA)
	bool isPhi() {return op == opPhi;}
	// True if this is a local variable
	bool isLocal() {return op == opLocal;}
	// True if this is a global variable
	bool isGlobal() {return op == opGlobal;}
	// True if this is a typeof
	bool isTypeOf() {return op == opTypeOf;}
	// Get the index for this var
	int getVarIndex();
	// True if this is a terminal
	virtual bool isTerminal() { return false; }
	// True if this is the constant "true"
	bool isTrue() {return op == opTrue;}
	// True if this is the constant "false"
	bool isFalse() {return op == opFalse;}
	// True if this is a disjunction, i.e. x or y
	bool isDisjunction() {return op == opOr;}
	// True if this is a conjunction, i.e. x and y
	bool isConjunction() {return op == opAnd;}
	// True if this is a boolean constant
	bool isBoolConst() {return op == opTrue || op == opFalse;}
	// True if this is an equality (== or !=)
	bool isEquality() {return op == opEquals /*|| op == opNotEqual*/;}
	// True if this is a comparison
	bool isComparison() { return op == opEquals || op == opNotEqual ||
								 op == opGtr || op == opLess ||
								 op == opGtrUns || op == opLessUns ||
								 op == opGtrEq || op == opLessEq ||
								 op == opGtrEqUns || op == opLessEqUns; }
	// True if this is a TypeVal
	bool isTypeVal() { return op == opTypeVal;}
	// True if this is a machine feature
	bool isMachFtr() {return op == opMachFtr;}
	// True if this is a location
	bool isLocation() { return op == opMemOf || op == opRegOf ||
							   op == opGlobal || op == opLocal ||
							   op == opParam; }
				 

	// Matches this expression to the pattern, if successful returns
	// a list of variable bindings, otherwise returns NULL
	virtual Exp *match(Exp *pattern);

	//	//	//	//	//	//	//
	//	Search and Replace	//
	//	//	//	//	//	//	//
	
	// Search for Exp *search in this Exp. If found, return true and return
	// a ref to the matching expression in result (useful with wildcards).
	virtual bool search(Exp* search, Exp*& result);

	// Search for Exp search in this Exp. For each found, add
	// a ptr to the matching expression in result (useful with wildcards).	  
	// Does NOT clear result on entry
	bool	searchAll(Exp* search, std::list<Exp*>& result);

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

	//	//	//	//	//	//	//
	//	  Sub expressions	//
	//	//	//	//	//	//	//
	
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

	// Get the memory nesting depth. Non mem-ofs return 0; m[m[x]] returns 2
virtual int getMemDepth() {return 0;}

	//	//	//	//	//	//	//
	//	Guarded assignment	//
	//	//	//	//	//	//	//
	Exp*	getGuard();			// Get the guard expression, or 0 if not
	
	//	//	//	//	//	//	//	//	//
	//	Expression Simplification	//
	//	//	//	//	//	//	//	//	//
	
	void	partitionTerms(std::list<Exp*>& positives, std::list<Exp*>& negatives,
		std::vector<int>& integers, bool negate);
virtual Exp*	simplifyArith() {return this;}
static Exp* Accumulate(std::list<Exp*> exprs);
	// Simplify the expression
	Exp*	simplify();
virtual Exp* polySimplify(bool& bMod) {bMod = false; return this;}
	// Just the address simplification a[ m[ any ]]
virtual Exp* simplifyAddr() {return this;}
virtual Exp* simplifyConstraint() {return this;}
		Exp* fixSuccessor();		// succ(r2) -> r3
		// Kill any zero fill, sign extend, or truncates
		Exp* killFill();

	// Do the work of finding used locations
			void addUsedLocs(LocationSet& used);

	Exp *removeSubscripts(bool& allZero);

	// Get number of definitions (statements this expression depends on)
virtual int getNumRefs() {return 0;}

	// Convert from SSA form
virtual Exp* fromSSA(igraph& ig) {return this;}

	// Convert from SSA form, where this is not subscripted (but defined at
	// statement d)
	Exp* fromSSAleft(igraph& ig, Statement* d);

	// Generate constraints for this Exp. NOTE: The behaviour is a bit different
	// depending on whether or not parameter result is a type constant or a
	// type variable.
	// If the constraint is always satisfied, return true
	// If the constraint can never be satisfied, return false
	// Example: this is opMinus and result is <int>, constraints are:
	//	 sub1 = <int> and sub2 = <int> or
	//	 sub1 = <ptr> and sub2 = <ptr>
	// Example: this is opMinus and result is Tr (typeOf r), constraints are:
	//	 sub1 = <int> and sub2 = <int> and Tr = <int> or
	//	 sub1 = <ptr> and sub2 = <ptr> and Tr = <int> or
	//	 sub1 = <ptr> and sub2 = <int> and Tr = <ptr>
virtual Exp*	genConstraints(Exp* result);

virtual Type	*getType() { return NULL; }

	// Visitation
	// Note: best to have accept() as pure virtual, so you don't forget to
	// implement it for new subclasses of Exp
virtual bool	accept(ExpVisitor* v) = 0;
virtual Exp*	accept(ExpModifier* v) = 0;
	void		fixLocationProc(UserProc* p);
	UserProc*	findProc();
	// Set or clear the constant subscripts
	void		setConscripts(int n, bool bClear);
	Exp*		stripRefs();			// Strip all references
	Exp*		stripSizes();			// Strip all size casts
	// Subscript all e in this Exp with statement def:
	Exp*		expSubscriptVar(Exp* e, Statement* def);
virtual Memo	*makeMemo(int mId) = 0;
virtual void	readMemo(Memo *m, bool dec) = 0;

protected:
	friend class XMLProgParser;
};	// Class Exp

// Not part of the Exp class, but logically belongs with it:
std::ostream& operator<<(std::ostream& os, Exp* p);	 // Print the Exp poited to by p

/*==============================================================================
 * Const is a subclass of Exp, and holds either an integer, floating point,
 * string, or address constant
 *============================================================================*/
class Const : public Exp {
	union {
		int i;			// Integer
		// Note: although we have i and a as unions, both often use the same
		// operator (opIntConst). There is no opCodeAddr any more.
		ADDRESS a;		// void* conflated with unsigned int: needs fixing
		QWord ll;	// 64 bit integer
		double d;		// Double precision float
		char* p;		// Pointer to string
						// Don't store string: function could be renamed
		Proc* pp;		// Pointer to function
	} u;
	int conscript;		// like a subscript for constants
public:
	// Special constructors overloaded for the various constants
			Const(int i);
			Const(QWord ll);
			Const(ADDRESS a);
			Const(double d);
			Const(char* p);
			Const(Proc* p);
	// Copy constructor
			Const(Const& o);
			
	// Nothing to destruct: Don't deallocate the string passed to constructor

	// Clone
	virtual Exp* clone();

	// Compare
virtual bool operator==(const Exp& o) const;
virtual bool operator< (const Exp& o) const;
virtual bool operator*=(Exp& o);

	// Get the constant
	int		getInt() {return u.i;}
	QWord	getLong(){return u.ll;}
	double	getFlt() {return u.d;}
	char*	getStr() {return u.p;}
	ADDRESS getAddr() {return u.a;}
const char*	getFuncName();

	// Set the constant
	void	setInt(int i)		{u.i = i;}
	void	setLong(QWord ll) {u.ll = ll;}
	void	setFlt(double d)	{u.d = d;}
	void	setStr(char* p)	{u.p = p;}
	void	setAddr(ADDRESS a) {u.a = a;}

virtual void print(std::ostream& os);
	// Print "recursive" (extra parens not wanted at outer levels)
		void printNoQuotes(std::ostream& os);
virtual void printx(int ind);
 

virtual void appendDotFile(std::ofstream& of);
virtual Exp* genConstraints(Exp* restrictTo);

	// Visitation
virtual bool accept(ExpVisitor* v);
virtual Exp* accept(ExpModifier* v);

	int		getConscript() {return conscript;}
	void	setConscript(int cs) {conscript = cs;}

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
};	// class Const

/*==============================================================================
 * Terminal is a subclass of Exp, and holds special zero arity items such
 * as opFlags (abstract flags register)
 *============================================================================*/
class Terminal : public Exp {
public:
	// Constructors
		Terminal(OPER op);
		Terminal(Terminal& o);		// Copy constructor

	// Clone
virtual Exp*	clone();

	// Compare
virtual bool	operator==(const Exp& o) const;
virtual bool	operator< (const Exp& o) const;
virtual bool	operator*=(Exp& o);

virtual void	print(std::ostream& os);
virtual void	appendDotFile(std::ofstream& of);
virtual void	printx(int ind);

virtual bool	isTerminal() { return true; }

	// Visitation
virtual bool 	accept(ExpVisitor* v);
virtual Exp*	accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
};	// class Terminal

/*==============================================================================
 * Unary is a subclass of Exp, holding one subexpression
 *============================================================================*/
class Unary : public Exp {
protected:
	Exp*		subExp1;	// One subexpression pointer
public:
	// Constructor, with just ID
			Unary(OPER op);
	// Constructor, with ID and subexpression
			Unary(OPER op, Exp* e);
	// Copy constructor
			Unary(Unary& o);

	// Clone
virtual Exp*	clone();

	// Compare
virtual bool	operator==(const Exp& o) const;
virtual bool	operator< (const Exp& o) const;
virtual bool	operator*=(Exp& o);

	// Destructor
virtual			~Unary();

	// Arity
virtual int		getArity() {return 1;}

	// Print
virtual void	print(std::ostream& os);
virtual void	appendDotFile(std::ofstream& of);
virtual void	printx(int ind);

	// Set first subexpression
	void		setSubExp1(Exp* e);
	void		setSubExp1ND(Exp* e) {subExp1 = e;}
	// Get first subexpression
	Exp*		getSubExp1();
	// "Become" subexpression 1 (delete all but that subexpression)
	Exp*		becomeSubExp1();
	// Get a reference to subexpression 1
	Exp*&		refSubExp1();
virtual int		getMemDepth();

virtual Exp*	match(Exp *pattern); 
		
	// Search children
	void 		doSearchChildren(Exp* search, std::list<Exp**>& li, bool once);

	// Do the work of simplifying this expression
virtual Exp*	polySimplify(bool& bMod);
		Exp*	simplifyArith();
		Exp*	simplifyAddr();
virtual Exp*	simplifyConstraint();

	// Convert from SSA form
virtual Exp*	fromSSA(igraph& ig);

	// Type analysis
virtual Exp*	genConstraints(Exp* restrictTo);

virtual Type*	getType();

	// Visitation
virtual bool	accept(ExpVisitor* v);
virtual Exp*	accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
};	// class Unary

/*==============================================================================
 * Binary is a subclass of Unary, holding two subexpressions
 *============================================================================*/
class Binary : public Unary {
protected:
	Exp*		subExp2;	// Second subexpression pointer
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
virtual bool operator==(const Exp& o) const ;
virtual bool operator< (const Exp& o) const ;
virtual bool operator*=(Exp& o);

	// Destructor
virtual		~Binary();

	// Arity
	int getArity() {return 2;}

	// Print
virtual void print(std::ostream& os);
virtual void printr(std::ostream& os);
virtual void appendDotFile(std::ofstream& of);
virtual void printx(int ind);

	// Set second subexpression
	void	setSubExp2(Exp* e);
	// Get second subexpression
	Exp*	getSubExp2();
	// "Become" subexpression 2 (delete all but that subexpression)
	Exp*	becomeSubExp2();
	// Commute the two operands
	void	commute();
	// Get a reference to subexpression 2
	Exp*&	refSubExp2();
virtual int getMemDepth();

virtual Exp* match(Exp *pattern); 

	// Search children
	void doSearchChildren(Exp* search, std::list<Exp**>& li, bool once);

	// Do the work of simplifying this expression
virtual Exp* polySimplify(bool& bMod);
	Exp*	simplifyArith();
	Exp*	simplifyAddr();
virtual Exp* simplifyConstraint();

	// Type analysis
virtual Exp* genConstraints(Exp* restrictTo);

	// Convert from SSA form
virtual Exp* fromSSA(igraph& ig);

virtual Type* getType();

	// Visitation
virtual bool accept(ExpVisitor* v);
virtual Exp* accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

private:
	Exp* constrainSub(TypeVal* typeVal1, TypeVal* typeVal2);

protected:
	friend class XMLProgParser;
};	// class Binary

/*==============================================================================
 * Ternary is a subclass of Binary, holding three subexpressions
 *============================================================================*/
class Ternary : public Binary {
	Exp*		subExp3;	// Third subexpression pointer
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
virtual bool operator==(const Exp& o) const ;
virtual bool operator< (const Exp& o) const ;
virtual bool operator*=(Exp& o);

	// Destructor
virtual		~Ternary();

	// Arity
	int		getArity() {return 3;}

	// Print
virtual void print(std::ostream& os);
virtual void printr(std::ostream& os);
virtual void appendDotFile(std::ofstream& of);
virtual void printx(int ind);

	// Set third subexpression
	void	setSubExp3(Exp* e);
	// Get third subexpression
	Exp*	getSubExp3();
	// "Become" subexpression 3 (delete all but that subexpression)
	Exp*	becomeSubExp3();
	// Get a reference to subexpression 3
	Exp*&	refSubExp3();
virtual int	getMemDepth();

	// Search children
	void	doSearchChildren(Exp* search, std::list<Exp**>& li, bool once);

virtual Exp* polySimplify(bool& bMod);
	Exp*	simplifyArith();
	Exp*	simplifyAddr();

	// Type analysis
virtual Exp* genConstraints(Exp* restrictTo);

	// Convert from SSA form
virtual Exp* fromSSA(igraph& ig);

virtual Type* getType();

	// Visitation
virtual bool	accept(ExpVisitor* v);
virtual Exp*	accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
};	// class Ternary

/*==============================================================================
 * TypedExp is a subclass of Unary, holding one subexpression and a Type
 *============================================================================*/
class TypedExp : public Unary {
	Type		*type;
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
virtual bool	operator==(const Exp& o) const;
virtual bool	operator%=(const Exp& o) const;		// Type insensitive compare
virtual bool	operator-=(const Exp& o) const;		// Sign insensitive compare
virtual bool	operator< (const Exp& o) const;
virtual bool	operator<<(const Exp& o) const;
virtual bool	operator*=(Exp& o);


virtual void	print(std::ostream& os);
virtual void	appendDotFile(std::ofstream& of);
virtual void	printx(int ind);

	// Get and set the type
virtual Type*	getType();
virtual void	setType(Type* ty);

	// polySimplify
virtual Exp*	polySimplify(bool& bMod);

	// Visitation
virtual bool	accept(ExpVisitor* v);
virtual Exp*	accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
};	// class TypedExp

/*==============================================================================
 * FlagDef is a subclass of Unary, and holds a list of parameters (in the
 *	subexpression), and a pointer to an RTL
 *============================================================================*/
class FlagDef : public Unary {
	RTL*		rtl;
public:
				FlagDef(Exp* params, RTL* rtl);		// Constructor
virtual			~FlagDef();							// Destructor
virtual void	appendDotFile(std::ofstream& of);
	RTL*		getRtl() { return rtl; }
	void		setRtl(RTL* r) { rtl = r; }

	// Visitation
virtual bool	accept(ExpVisitor* v);
virtual Exp*	accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
};	// class FlagDef

/*==============================================================================
 * RefExp is a subclass of Unary, holding an ordinary Exp pointer, and
 *	a pointer to a defining statement (could be a phi assignment)
 * This is used for subscripting SSA variables. Example:
 * m[1000] becomes m[1000]{3} if defined at statement 3
 * The integer is really a pointer to the definig statement,
 * printed as the statement number for compactness
 *============================================================================*/
class RefExp : public Unary {
	Statement* def;				// The defining statement

public:
				// Constructor with expression (e) and statement defining it (def)
				RefExp(Exp* e, Statement* def);
				//RefExp(Exp* e);
				//RefExp(RefExp& o);
virtual Exp* 	clone();
virtual bool 	operator==(const Exp& o) const;
virtual bool 	operator< (const Exp& o) const;
virtual bool	operator*=(Exp& o);

virtual void	print(std::ostream& os);
virtual void	printx(int ind);
virtual int		getNumRefs() {return 1;}
	Statement*	getRef() {return def;}
	Exp*		addSubscript(Statement* def) {this->def = def; return this;}
	void		setDef(Statement* def) {this->def = def;}
virtual Exp*	genConstraints(Exp* restrictTo);
virtual Exp*	fromSSA(igraph& ig);
	bool		references(Statement* s) {return def == s;}
virtual Exp*	polySimplify(bool& bMod);
virtual Type*	getType();
virtual Exp		*match(Exp *pattern);

	// Visitation
virtual bool accept(ExpVisitor* v);
virtual Exp* accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	RefExp() : Unary(opSubscript), def(NULL) { }
	friend class XMLProgParser;
};	// Class RefExp

#if 0
/*==============================================================================
 * PhiExp is a subclass of Unary, holding an operator (opPhi), the expression
 * that is being phi'd (in subExp1), and a StatementVec
 * This is used for phi functions. For example:
 * m[1000] := phi{3 7 10} if defined at statements 3, 7, and 10
 * m[r28{3}+4] := phi{2 8} if the memof is defined at 2 and 8, and
 * the r28 is defined at 3. The integers are really pointers to statements,
 * printed as the statement number for compactness
 * NOTE: Although the subexpression is nearly always redundant, it is needed
 * in one circumstance: when finding locations used by this statement, and
 * the reference is to a CallStatement returning multiple locations.
 *============================================================================*/
class PhiExp : public Unary {
	// A vector is used below so that it can be determined which in-edge
	// corresponds to each phi parameter (member of stmtVec)
	// The first entry is for the first in-edge, and so on
	// If the last few in-edges have no definitions, there need not be
	// members of stmtVec for them
	StatementVec	stmtVec;		// A vector of pointers to statements
	Assign *stmt;

public:
			PhiExp(Exp* e) : Unary(opPhi, e) {};
			// Constructor with statement defining it (def)
			PhiExp(Exp* e, Statement* def);
			PhiExp(PhiExp& o);
virtual Exp* clone();
virtual bool operator==(const Exp& o) const;
virtual bool operator< (const Exp& o) const;
virtual bool operator*=(Exp& o);
virtual void	print(std::ostream& os, bool withUses = false);
virtual int getNumRefs() {return stmtVec.size();}
	bool	hasGlobalFuncParam(Prog *prog);
virtual Exp*   addSubscript(Statement* def) {assert(0); return NULL; }
	Statement* getAt(int idx) {return stmtVec.getAt(idx);}
	void	   putAt(int idx, Statement* d) {stmtVec.putAt(idx, d);}
	//Statement* getFirstRef(StmtVecIter& it) {return stmtVec.getFirst(it);}
	//Statement* getNextRef (StmtVecIter& it) {return stmtVec.getNext (it);}
	//bool		 isLastRef(StmtVecIter& it) {return stmtVec.isLast(it);}
	StatementVec::iterator begin() {return stmtVec.begin();}
	StatementVec::iterator end()   {return stmtVec.end();}
virtual Exp* fromSSA(igraph& ig);
	//bool	  references(Statement* s) {return stmtVec.exists(s);}
	StatementVec& getRefs() {return stmtVec;}
virtual Exp*  genConstraints(Exp* restrictTo);
	void	setStatement(Assign *a) { stmt = a; }

	// polySimplify
virtual Exp* polySimplify(bool& bMod);
	void simplifyRefs();

	// Visitation
virtual bool accept(ExpVisitor* v);
virtual Exp* accept(ExpModifier* v);

protected:
	PhiExp() : Unary(opPhi) { }
	friend class XMLProgParser;
};	// class PhiExp
#endif

/*==============================================================================
class TypeVal. Just a Terminal with a Type. Used for type values in constraints
==============================================================================*/
class TypeVal : public Terminal {
	Type*	val;

public:
	TypeVal(Type* ty);
	~TypeVal();

virtual Type*	getType() {return val;}
virtual void	setType(Type* t) {val = t;}
virtual Exp*	clone();
virtual bool	operator==(const Exp& o) const;
virtual bool	operator< (const Exp& o) const;
virtual bool	operator*=(Exp& o);
virtual void	print(std::ostream& os);
virtual void	printx(int ind);
virtual Exp* 	genConstraints(Exp* restrictTo) {
					assert(0); return NULL;} // Should not be constraining constraints
virtual Exp		*match(Exp *pattern);

	// Visitation
virtual bool	accept(ExpVisitor* v);
virtual Exp*	accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
};	// class TypeVal

class Location : public Unary {
protected:
	UserProc *proc;
	Type *ty;

public:
	// Constructor with ID, subexpression, and UserProc*
				Location(OPER op, Exp* e, UserProc *proc);
	// Copy constructor
				Location(Location& o);
	// Custom constructor
static Location* regOf(int r) {return new Location(opRegOf, new Const(r), NULL);}
static Location* regOf(Exp *e) {return new Location(opRegOf, e, NULL);}
static Location* memOf(Exp *e, UserProc* p = NULL) {return new Location(opMemOf, e, p);}
static Location* tempOf(Exp* e) {return new Location(opTemp, e, NULL);}
static Location* global(const char *nam, UserProc *p) {
					return new Location(opGlobal, new Const((char*)nam), p);}
static Location* local(const char *nam, UserProc *p) {
					return new Location(opLocal, new Const((char*)nam), p);}
static Location* param(const char *nam, UserProc *p = NULL) {
					return new Location(opParam, new Const((char*)nam), p);}
	// Clone
virtual Exp*	clone();

	void		setProc(UserProc *p) { proc = p; }
	UserProc	*getProc() { return proc; }

virtual Exp*	polySimplify(bool& bMod);
virtual void	getDefinitions(LocationSet& defs);

virtual Type	*getType();
virtual void	setType(Type *t) { ty = t; }
virtual int		getMemDepth();

	// Visitation
virtual bool	accept(ExpVisitor* v);
virtual Exp*	accept(ExpModifier* v);

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

protected:
	friend class XMLProgParser;
				Location(OPER op) : Unary(op), proc(NULL), ty(NULL) { }
};	// Class Location
	
#endif // __EXP_H__
