/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   statement.h
 * OVERVIEW:   The Statement and related classes (was dataflow.h)
 *============================================================================*/

/*
 * $Revision$
 * 25 Nov 02 - Trent: appropriated for use by new dataflow.
 * 3 July 02 - Trent: created.
 * 03 Feb 03 - Mike: cached dataflow (uses and usedBy)
 * 03 Apr 03 - Mike: Added StatementSet
 * 25 Jul 03 - Mike: Changed dataflow.h to statement.h
 * 15 Jul 04 - Mike: New Assignment hierarchy
 * 11 Aug 04 - Mike: BoolStatement -> BoolAssign
 * 17 Sep 04 - Mike: PhiExp in ordinary assignment replaced by PhiAssign statement
 * 27 Oct 04 - Mike: PhiAssign has vector of PhiInfo now; needed because a statement pointer alone does not uniquely
 *						define what is defined. It is now possible for all parameters of a phi to have different exps
 */

#ifndef _STATEMENT_H_
#define _STATEMENT_H_

/* Class hierarchy:		 Statement (abstract)
						 /		 \
						/		  \
			 GotoStatement	Assignment (abstract)
 BranchStatement__/			/	/|\	  \
 ReturnStatement_/	   Assign  / | \  PhiAssign
 CallStatement__/ ImplicitAssign |  BoolAssign
 CaseStatement_/			SigmaAssign (soon)
*/

#include <vector>
#include <set>
#include <list>
#include <map>
#include <ostream>
#include <iostream>		// For std::cerr
#include <assert.h>
//#include "exp.h"		// No! This is the bottom of the #include hierarchy
#include "exphelp.h"	// For lessExpStar
#include "types.h"
#include "managed.h"

class BasicBlock;
typedef BasicBlock *PBB;
class Prog;
class Proc;
class UserProc;
class Exp;
class Const;
class RefExp;
class Cfg;
class Type;
class Statement;
class StmtVisitor;
class StmtExpVisitor;
class StmtModifier;
class HLLCode;
class Assign;
class RTL;
class XMLProgParser;

// The map of interferences. It maps locations such as argc{55} to a local, e.g. local17
typedef std::map<Exp*, Exp*, lessExpStar> igraph;

/*==============================================================================
 * Kinds of Statements, or high-level register transfer lists.
 * changing the order of these will result in save files not working - trent
 *============================================================================*/
enum STMT_KIND {
	STMT_ASSIGN = 0,
	STMT_PHIASSIGN,
	STMT_IMPASSIGN,
	STMT_BOOLASSIGN,				// For "setCC" instructions that set destination
	STMT_CALL,
	STMT_RET,
	STMT_BRANCH,
	STMT_GOTO,
								// to 1 or 0 depending on the condition codes.
	STMT_CASE,					// Used to represent switch statements.
};

/*==============================================================================
 * BRANCH_TYPE: These values indicate what kind of conditional jump or
 * conditonal assign is being performed.
 * Changing the order of these will result in save files not working - trent
 *============================================================================*/
enum BRANCH_TYPE {
	BRANCH_JE = 0,			// Jump if equals
	BRANCH_JNE,				// Jump if not equals
	BRANCH_JSL,				// Jump if signed less
	BRANCH_JSLE,			// Jump if signed less or equal
	BRANCH_JSGE,			// Jump if signed greater or equal
	BRANCH_JSG,				// Jump if signed greater
	BRANCH_JUL,				// Jump if unsigned less
	BRANCH_JULE,			// Jump if unsigned less or equal
	BRANCH_JUGE,			// Jump if unsigned greater or equal
	BRANCH_JUG,				// Jump if unsigned greater
	BRANCH_JMI,				// Jump if result is minus
	BRANCH_JPOS,			// Jump if result is positive
	BRANCH_JOF,				// Jump if overflow
	BRANCH_JNOF,			// Jump if no overflow
	BRANCH_JPAR				// Jump if parity even (Intel only)
};

//	//	//	//	//	//	//	//	//	//	//	//	//	//
//
//	A b s t r a c t	  C l a s s	  S t a t e m e n t //
//
//	//	//	//	//	//	//	//	//	//	//	//	//	//

/* Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 */
class Statement {
protected:
		PBB			pbb;			// contains a pointer to the enclosing BB
		UserProc	*proc;			// procedure containing this statement
		int			number;			// Statement number for printing
		STMT_KIND	kind;			// Statement kind (e.g. STMT_BRANCH)
		Statement	*parent;		// The statement that contains this one

		unsigned int lexBegin, lexEnd;

public:

					Statement() : pbb(NULL), proc(NULL), number(0), parent(NULL) { }
virtual				~Statement() { }

		// get/set the enclosing BB, etc
		PBB			getBB() { return pbb; }
		void		setBB(PBB bb) { pbb = bb; }

		bool		operator==(Statement& o);
		// Get and set *enclosing* proc (not destination proc)
		void		setProc(UserProc *p);
		UserProc*	getProc() {return proc;}

		int			getNumber() {return number;}
		void		setNumber(int num) {number = num;}

		STMT_KIND	getKind() { return kind;}
		void		setKind(STMT_KIND k) {kind = k;}

		void		setParent(Statement* par) {parent = par;}
		Statement*	getParent() {return parent;}

virtual Statement*	clone() = 0;			   // Make copy of self

	// Accept a visitor (of various kinds) to this Statement. Return true to continue visiting
virtual bool		accept(StmtVisitor* visitor) = 0;
virtual bool		accept(StmtExpVisitor* visitor) = 0;
virtual bool		accept(StmtModifier* visitor) = 0;

		void		setLexBegin(unsigned int n) { lexBegin = n; }
		void		setLexEnd(unsigned int n) { lexEnd = n; }
		unsigned	int getLexBegin() { return lexBegin; }
		unsigned	int getLexEnd() { return lexEnd; }
		Exp			*getExpAtLex(unsigned int begin, unsigned int end);


		// returns true if this statement defines anything
virtual bool		isDefinition() = 0;

		// true if is a null statement
		bool		isNullStatement();

		// true if this statement is a standard assign
		bool		isAssign() {return kind == STMT_ASSIGN;}
		// true if this statement is a any kind of assignment
		bool		isAssignment() {return kind == STMT_ASSIGN || kind == STMT_PHIASSIGN ||
						kind == STMT_IMPASSIGN || kind == STMT_BOOLASSIGN;}
		// true	if this statement is a phi assignment
		bool		isPhi() {return kind == STMT_PHIASSIGN; }
		// true	if this statement is an implicit assignment
		bool		isImplicit() {return kind == STMT_IMPASSIGN;}
		// true	if this statment is a flags assignment
		bool		isFlagAssgn();

virtual bool		isGoto() { return kind == STMT_GOTO; }
virtual bool		isBranch() { return kind == STMT_BRANCH; }

		// true if this statement is a call
		bool		isCall() { return kind == STMT_CALL; }

		// true if this statement is a BoolAssign
		bool		isBool() { return kind == STMT_BOOLASSIGN; }

		// true if this statement is a ReturnStatement
		bool		isReturn() { return kind == STMT_RET; }

		// true if this is a fpush/fpop
		bool		isFpush();
		bool		isFpop();

		// returns a set of locations defined by this statement
		// Classes with no definitions (e.g. GotoStatement and children) don't
		// override this
virtual void		getDefinitions(LocationSet &def) {}

		// returns an expression that would be used to reference the value
		// defined by this statement (if this statement is propogatable)
virtual Exp*		getLeft() = 0;
		// set the left for forExp to newExp
virtual	void		setLeftFor(Exp* forExp, Exp* newExp) {assert(0);}

	// returns an expression that would be used to replace this statement in a use
virtual Exp*		getRight() = 0;

	// returns true if this statement uses the given expression
virtual bool		usesExp(Exp *e) = 0;

	// statements should be printable (for debugging)
virtual void		print(std::ostream &os) = 0;
		void		printAsUse(std::ostream &os)   {os << std::dec << number;}
		void		printAsUseBy(std::ostream &os) {os << std::dec << number;}
		void		printNum(std::ostream &os)	   {os << std::dec << number;}
		char*		prints();	   // For use in a debugger

		// inline / decode any constants in the statement
virtual void		processConstants(Prog *prog) = 0;

		// general search
virtual bool		search(Exp *search, Exp *&result) = 0;
virtual bool		searchAll(Exp* search, std::list<Exp*>& result) = 0;

		// general search and replace
virtual bool		searchAndReplace(Exp *search, Exp *replace) = 0;

		// From SSA form
virtual void		fromSSAform(igraph& igm) = 0;

		// Propagate to this statement
		bool		propagateTo(int memDepth, StatementSet& exclude, int toDepth = -1, bool limit = true);

		// code generation
virtual void		generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) = 0;

		// simpify internal expressions
virtual void		simplify() = 0;

		// simplify internal address expressions (a[m[x]] -> x)
		// Only Assigns override at present
virtual void		simplifyAddr() {}

		// fixSuccessor
		// Only Assign overrides at present
virtual void		fixSuccessor() {}

		// Generate constraints (for constraint based type analysis)
virtual void		genConstraints(LocationSet& cons) {}

		// Data flow based type analysis
virtual	void		dfaTypeAnalysis(bool& ch) {}			// Use the type information in this Statement
		Type*		meetWithFor(Type* ty, Exp* e, bool& ch);// Meet the type associated with e with ty

		// Replace registers with locals
virtual	void		regReplace(UserProc* proc) = 0;

//	//	//	//	//	//	//	//	//	//
//									//
//	Statement visitation functions	//
//									//
//	//	//	//	//	//	//	//	//	//

	// Adds (inserts) all locations (registers or memory etc) used by this
	// statement
		void		addUsedLocs(LocationSet& used, bool final = false);
		void		fixCallRefs();


		// replaces a use of the given statement with an expression
		bool		replaceRef(Statement *use);
		// special version of the above for the "special hack"
		// (see Proc::propagateStatements, where numUses == 2)
		void		specialReplaceRef(Statement* def);

		// Find all constants in this statement
		void		findConstants(std::list<Const*>& lc);

		// Set or clear the constant subscripts (using a visitor)
		int			setConscripts(int n);
		void		clearConscripts();

		// Strip all references and phis (using a visitor). Returns true if the
		// statement was a phi (and to be deleted)
		bool		stripRefs();

		// Strip all size casts
		void		stripSizes();

		// For all expressions in this Statement, replace all e with e{def}
		void		subscriptVar(Exp* e, Statement* def /*, Cfg* cfg */);

		// Cast the constant num to type ty. If a change was made, return true
		bool		castConst(int num, Type* ty);

		// Convert expressions to locals
		void		dfaConvertLocals();

		// End Statement visitation functions


		// Get the type for the given expression in this statement. This is an old version for processConstants
		Type		*getTypeFor(Exp *e, Prog *prog);

		// Get the type for the definition, if any, for expression e in this statement 
		// Overridden only by Assignment and CallStatement. (ReturnStatements do not define anything.)
virtual	Type*		getTypeFor(Exp* e) { return NULL;}
		// Set the type for the definition of e in this Statement
virtual	void		setTypeFor(Exp* e, Type* ty) {assert(0);}

//virtual	Type*	getType() {return NULL;}			// Assignment, ReturnStatement and
//virtual	void	setType(Type* t) {assert(0);}		// CallStatement override

protected:
		// Returns true if an indirect call is converted to direct:
virtual bool		doReplaceRef(Exp* from, Exp* to) = 0;
		bool		doPropagateTo(int memDepth, Statement* def, bool& convert);
		bool		calcMayAlias(Exp *e1, Exp *e2, int size);
		bool		mayAlias(Exp *e1, Exp *e2, int size);

	friend class XMLProgParser;
};		// class Statement

// Print the Statement (etc) poited to by p
std::ostream& operator<<(std::ostream& os, Statement* p);
std::ostream& operator<<(std::ostream& os, StatementSet* p);
std::ostream& operator<<(std::ostream& os, LocationSet* p);



/*==============================================================================
 * Assignment is an abstract subclass of Statement, holding a location and a
 * Type
 *============================================================================*/
class Assignment : public Statement {
protected:
	Type*		type;		// The type for this assignment
	Exp*		lhs;		// The left hand side
public:
	// Constructor, subexpression
				Assignment(Exp* lhs);
	// Constructor, type, and subexpression
				Assignment(Type* ty, Exp* lhs);
	// Destructor
virtual			~Assignment();

	// Clone
virtual Statement* clone() = 0;

	// Accept a visitor to this Statement
virtual bool	accept(StmtVisitor* visitor) = 0;
virtual bool	accept(StmtExpVisitor* visitor) = 0;
virtual bool	accept(StmtModifier* visitor) = 0;

virtual void	print(std::ostream& os) = 0;

virtual Type*	getTypeFor(Exp* e); 			// Get the type for this assignment. It should define e
virtual void	setTypeFor(Exp* e, Type* ty); 	// Set the type for this assignment. It should define e

		// Get and set the type. Not polymorphic (any more)
		Type*	getType() {return type;}
		void	setType(Type* ty) {type = ty;}

virtual bool	usesExp(Exp *e);	   // PhiAssign and ImplicitAssign don't override

virtual bool	isDefinition() { return true; }
virtual void	getDefinitions(LocationSet &defs);
		
	// get how to access this lvalue
virtual Exp*		getLeft() { return lhs; }
virtual	void		setLeftFor(Exp* forExp, Exp* newExp) {lhs = newExp; }

		// set the lhs to something new
		void	setLeft(Exp* e)	 { lhs = e; }

		// memory depth
		int		getMemDepth();

		// from SSA form
virtual void	fromSSAform(igraph& ig);

		// general search
virtual bool	search(Exp *search, Exp *&result) = 0;
virtual bool	searchAll(Exp* search, std::list<Exp*>& result) = 0;

		// general search and replace
virtual bool	searchAndReplace(Exp *search, Exp *replace) = 0;

	void		generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {}

		// simpify internal expressions
virtual void	simplify() = 0;

		// generate Constraints
virtual void	genConstraints(LocationSet& cons);

		// Data flow based type analysis
		void	dfaTypeAnalysis(bool& ch);

		// Replace registers with locals
virtual	void	regReplace(UserProc* proc);

protected:
virtual bool	doReplaceRef(Exp* from, Exp* to);

	friend class XMLProgParser;
};		// class Assignment


// Assign: an ordinary assignment with left and right sides
class Assign : public Assignment {
	Exp*	rhs;
	Exp*	guard;

public:
	// Constructor, subexpressions
				Assign(Exp* lhs, Exp* rhs);
	// Constructor, type and subexpressions
				Assign(Type* ty, Exp* lhs, Exp* rhs);
	// Default constructor, for XML parser
				Assign() : Assignment(NULL), rhs(NULL), guard(NULL) {}
	// Copy constructor
				Assign(Assign& o);
	// Destructor
				~Assign() {}

	// Clone
virtual Statement* clone();

	// get how to replace this statement in a use
virtual Exp*	getRight() { return rhs; }

	// set the rhs to something new
	void		setRight(Exp* e) { rhs = e; }



	// Accept a visitor to this Statement
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

virtual void	print(std::ostream& os);

	// Guard
	void		setGuard(Exp* g) {guard = g;}
	Exp*		getGuard() {return guard;}
	bool		isGuarded() {return guard != NULL;}

virtual bool	usesExp(Exp *e);
virtual bool	isDefinition() { return true; }
		
	// inline any constants in the statement
virtual void	processConstants(Prog *prog);

	// general search
virtual bool	search(Exp* search, Exp*& result);
virtual bool	searchAll(Exp* search, std::list<Exp*>& result);

	// general search and replace
virtual bool	searchAndReplace(Exp *search, Exp *replace);
 
	// memory depth
	int			getMemDepth();

	// from SSA form
virtual void	fromSSAform(igraph& ig);

	// Generate code
virtual void	generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

	// simpify internal expressions
virtual void	simplify();

	// simplify address expressions
virtual void	simplifyAddr();

	// fixSuccessor (succ(r2) -> r3)
virtual void	fixSuccessor();

	// generate Constraints
virtual void	genConstraints(LocationSet& cons);

		// Data flow based type analysis
		void	dfaTypeAnalysis(bool& ch);

		// Replace registers with locals
virtual	void	regReplace(UserProc* proc);

protected:
virtual bool	doReplaceRef(Exp* from, Exp* to);
	friend class XMLProgParser;
};	// class Assign

/*==============================================================================
 * PhiExp is a subclass of Assignment, having a left hand side, and a
 * StatementVec with the references.
 * Example:
 * m[1000] := phi{3 7 10}	m[1000] is defined at statements 3, 7, and 10
 * m[r28{3}+4] := phi{2 8}	the memof is defined at 2 and 8, and
 * the r28 is defined at 3. The integers are really pointers to statements,
 * printed as the statement number for compactness
 * NOTE: Although the left hand side is nearly always redundant, it is essential
 * in at least one circumstance: when finding locations used by some statement,
 * and the reference is to a CallStatement returning multiple locations.
 * Besides, the lhs gives it useful common functionality with other Assignments
 *============================================================================*/
// The below could almost be a RefExp. But we can't #include exp.h, we don't need another copy of the Memo class,
// and it's more convenient to have these members public
struct PhiInfo {
		Statement*	def;		// The defining statement
		Exp*		e;			// The expression for the thing being defined (never subscripted)
};
class PhiAssign : public Assignment {
public:
		typedef std::vector<PhiInfo> Definitions;
		typedef Definitions::iterator iterator;
private:
	Definitions	defVec;		// A vector of information about definitions
public:
		// Constructor, subexpression
				PhiAssign(Exp* lhs)
				  : Assignment(lhs) {kind = STMT_PHIASSIGN;}
		// Constructor, type and subexpression
				PhiAssign(Type* ty, Exp* lhs)
				  : Assignment(ty, lhs) {kind = STMT_PHIASSIGN;}
		// Copy constructor (not currently used or implemented)
				PhiAssign(Assign& o);
		// Destructor
virtual			~PhiAssign() {}

		// Clone
virtual Statement* clone();

	// get how to replace this statement in a use
virtual Exp*	getRight() { return NULL; }

	// Accept a visitor to this Statement
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

virtual void	print(std::ostream& os);

	// inline any constants in the statement
virtual void	processConstants(Prog *prog);

	// general search
virtual bool	search(Exp* search, Exp*& result);
virtual bool	searchAll(Exp* search, std::list<Exp*>& result);

	// general search and replace
virtual bool	searchAndReplace(Exp *search, Exp *replace);
 
	// simplify all the uses/defs in this RTL
virtual void	simplify();

	// from SSA form
virtual void	fromSSAform(igraph& ig);

	// Generate constraints
virtual void	genConstraints(LocationSet& cons);

		// Data flow based type analysis
		void	dfaTypeAnalysis(bool& ch);

//
//	Phi specific functions
//

		// Get or put the statement at index idx
		Statement*	getStmtAt(int idx) {return defVec[idx].def;}
		PhiInfo&	getAt(int idx) {return defVec[idx];}
		void		putAt(int idx, Statement* d, Exp* e);
		void		simplifyRefs();
virtual	int			getNumDefs() {return defVec.size();}
		Definitions& getDefs() {return defVec;}
		// A hack. Check MVE
		bool		hasGlobalFuncParam();

		Definitions::iterator begin() {return defVec.begin();}
		Definitions::iterator end()   {return defVec.end();}

		// Convert this phi assignment to an ordinary assignment
		void		convertToAssign(Exp* rhs);

protected:
		friend class XMLProgParser;
};		// class PhiAssign

// An implicit assignment has only a left hand side. It is a placeholder for
// storing the types of parameters and globals
// That way, you can always find the type of a subscripted variable by
// looking in its defining Assignment
class ImplicitAssign : public Assignment {
public:
		// Constructor, subexpression
				ImplicitAssign(Exp* lhs) : Assignment(lhs) {kind = STMT_IMPASSIGN;}
		// Constructor, type, and subexpression
				ImplicitAssign(Type* ty, Exp* lhs) : Assignment(ty, lhs) {kind = STMT_IMPASSIGN;}
		// Copy constructor
				ImplicitAssign(ImplicitAssign& o);
		// Destructor
virtual			~ImplicitAssign();

		// Clone
virtual Statement* clone();

		// inline any constants in the statement
virtual void	processConstants(Prog *prog);

		// general search
virtual bool	search(Exp* search, Exp*& result);
virtual bool	searchAll(Exp* search, std::list<Exp*>& result);

		// general search and replace
virtual bool	searchAndReplace(Exp *search, Exp *replace);
 
virtual void	print(std::ostream& os);

		// Statement and Assignment functions
virtual Exp*	getRight() { return NULL; }
virtual void	simplify() {}

		// Visitation
virtual	bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

};		// class ImplicitAssign

/*==============================================================================
 * BoolAssign represents "setCC" type instructions, where some destination is
 * set (to 1 or 0) depending on the condition codes. It has a condition
 * Exp, similar to the BranchStatement class.
 * *==========================================================================*/
class BoolAssign: public Assignment {
		BRANCH_TYPE jtCond;		// the condition for setting true
		Exp*	pCond;			// Exp representation of the high level
								// condition: e.g. r[8] == 5
		bool	bFloat;			// True if condition uses floating point CC
		int		size;			// The size of the dest
public:
				BoolAssign(int size);
virtual			~BoolAssign();

		// Make a deep copy, and make the copy a derived object if needed.
virtual Statement* clone();

		// Accept a visitor to this Statement
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

		// Set and return the BRANCH_TYPE of this scond as well as whether the
		// floating point condition codes are used.
		void	setCondType(BRANCH_TYPE cond, bool usesFloat = false);
		BRANCH_TYPE getCond(){return jtCond;}
		bool	isFloat(){return bFloat;}
		void	setFloat(bool b) { bFloat = b; }

		// Set and return the Exp representing the HL condition
		Exp*	getCondExpr();
		void	setCondExpr(Exp* pss);
		// As above, no delete (for subscripting)
		void	setCondExprND(Exp* e) { pCond = e; }

		int		getSize() {return size;}	// Return the size of the assignment
		void	makeSigned();

virtual void	print(std::ostream& os = std::cout);

		// code generation
virtual void	generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

		// simplify all the uses/defs in this RTL
virtual void	simplify();

		// Statement functions
virtual bool	isDefinition() { return true; }
virtual void	getDefinitions(LocationSet &def);
virtual Exp*	getRight() { return getCondExpr(); }
virtual bool	usesExp(Exp *e);
virtual void	processConstants(Prog *prog);
virtual bool	search(Exp *search, Exp *&result);
virtual bool	searchAll(Exp* search, std::list<Exp*>& result);
virtual bool	searchAndReplace(Exp *search, Exp *replace);
virtual bool	doReplaceRef(Exp* from, Exp* to);
		// from SSA form
virtual void	fromSSAform(igraph& ig);
		// a hack for the SETS macro
		void	setLeftFromList(std::list<Statement*>* stmts);

virtual void	dfaTypeAnalysis(bool& ch);

	friend class XMLProgParser;
};	// class BoolAssign

/*=============================================================================
 * GotoStatement has just one member variable, an expression representing the
 * jump's destination (an integer constant for direct jumps; an expression
 * for register jumps). An instance of this class will never represent a
 * return or computed call as these are distinguised by the decoder and are
 * instantiated as CallStatements and ReturnStatements respecitvely.
 * This class also represents unconditional jumps with a fixed offset
 * (e.g BN, Ba on SPARC).
 *===========================================================================*/
class GotoStatement: public Statement {
protected:
	Exp*		pDest;			// Destination of a jump or call. This is the
								// absolute destination for both static and
								// dynamic CTIs.
	bool		m_isComputed;	// True if this is a CTI with a computed
								// destination address. NOTE: This should be
								// removed, once CaseStatement and HLNwayCall
								// are implemented properly.
public:
				GotoStatement();
				GotoStatement(ADDRESS jumpDest);
virtual			~GotoStatement();

	// Make a deep copy, and make the copy a derived object if needed.
virtual Statement* clone();

	// Accept a visitor to this Statement
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

	// Set and return the destination of the jump. The destination is either
	// a Exp, or an ADDRESS that is converted to a Exp.
	void		setDest(Exp* pd);
	void		setDest(ADDRESS addr);
virtual Exp*	getDest();

	// Return the fixed destination of this CTI. For dynamic CTIs, returns -1.
	ADDRESS		getFixedDest();

	// Adjust the fixed destination by a given amount. Invalid for dynamic CTIs.
	void		adjustFixedDest(int delta);
	
	// Set and return whether the destination of this CTI is computed.
	// NOTE: These should really be removed, once CaseStatement and HLNwayCall
	// are implemented properly.
	void		setIsComputed(bool b = true);
	bool		isComputed();

virtual void	print(std::ostream& os = std::cout);

	// general search
virtual bool	search(Exp*, Exp*&);

	// Replace all instances of "search" with "replace".
virtual bool	searchAndReplace(Exp* search, Exp* replace);
	
	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.	 
virtual bool	searchAll(Exp* search, std::list<Exp*> &result);

	// code generation
virtual void	generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

	// simplify all the uses/defs in this RTL
virtual void	simplify();

	// Statement virtual functions
virtual bool	isDefinition() { return false;}
virtual Exp*	getLeft() {return NULL;}
virtual Exp*	getRight() {return NULL;}
virtual bool	usesExp(Exp*) {return false;}
virtual void	processConstants(Prog*) {}
virtual void	fromSSAform(igraph&) {}
virtual bool	doReplaceRef(Exp*, Exp*) {return false;}

		// Replace registers with locals
virtual	void	regReplace(UserProc* proc);

	friend class XMLProgParser;
};		// class GotoStatement


/*==============================================================================
 * BranchStatement has a condition Exp in addition to the destination of the jump.
 *============================================================================*/
class BranchStatement: public GotoStatement {
	BRANCH_TYPE jtCond;			// The condition for jumping
	Exp*		pCond;			// The Exp representation of the high level
								// condition: e.g., r[8] == 5
	bool		bFloat;			// True if uses floating point CC

public:
				BranchStatement();
virtual			~BranchStatement();

	// Make a deep copy, and make the copy a derived object if needed.
virtual Statement* clone();

	// Accept a visitor to this RTL
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

	// Set and return the BRANCH_TYPE of this jcond as well as whether the
	// floating point condition codes are used.
	void		setCondType(BRANCH_TYPE cond, bool usesFloat = false);
	BRANCH_TYPE getCond(){ return jtCond; }
	bool		isFloat(){ return bFloat; }
	void		setFloat(bool b)	  { bFloat = b; }

	// Set and return the Exp representing the HL condition
	Exp*		getCondExpr();
	void		setCondExpr(Exp* pe);
	// As above, no delete (for subscripting)
	void		setCondExprND(Exp* e) { pCond = e; }
	
	// Probably only used in front386.cc: convert this from an unsigned to a
	// signed conditional branch
	void		makeSigned();

virtual void	print(std::ostream& os = std::cout);

	// general search
virtual bool	search(Exp *search, Exp *&result);

	// Replace all instances of "search" with "replace".
virtual bool	searchAndReplace(Exp* search, Exp* replace);
	
	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.
virtual bool	searchAll(Exp* search, std::list<Exp*> &result);

#if 0
	// Used for type analysis. Stores type information that
	// can be gathered from the RTL instruction inside a
	// data structure within BBBlock inBlock
	void		storeUseDefineStruct(BBBlock& inBlock);	  
#endif

		// code generation
virtual void	generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

		// dataflow analysis
virtual bool	usesExp(Exp *e);

		// dataflow related functions
virtual bool	canPropagateToAll() { return false; }
virtual void	propagateToAll() { assert(false); }

virtual bool	isDefinition() { return false; }

		// get how to access this value
virtual Exp*	getLeft() { return NULL; }

		// get how to replace this statement in a use
virtual Exp*	getRight() { return pCond; }

		// simplify all the uses/defs in this RTL
virtual void	simplify();

		// From SSA form
virtual void	fromSSAform(igraph& ig);

		// Generate constraints
virtual void	genConstraints(LocationSet& cons);

		// Data flow based type analysis
		void	dfaTypeAnalysis(bool& ch);

		// Replace registers with locals
virtual	void	regReplace(UserProc* proc);

protected:
virtual bool	doReplaceRef(Exp* from, Exp* to);

	friend class XMLProgParser;
};		// class BranchStatement

/*==============================================================================
 * CaseStatement is derived from GotoStatement. In addition to the destination
 * of the jump, it has a switch variable Exp.
 *============================================================================*/
typedef struct {
	Exp*		pSwitchVar;		// Ptr to Exp repres switch var, e.g. v[7]
	char		chForm;			// Switch form: 'A', 'O', 'R', or 'H'
	int			iLower;			// Lower bound of the switch variable
	int			iUpper;			// Upper bound for the switch variable
	ADDRESS		uTable;			// Native address of the table
	int			iNumTable;		// Number of entries in the table (form H only)
	int			iOffset;		// Distance from jump to table (form R only)
//	int			delta;			// Host address - Native address
} SWITCH_INFO;

class CaseStatement: public GotoStatement {
	SWITCH_INFO* pSwitchInfo;	// Ptr to struct with info about the switch
public:
				CaseStatement();
virtual			~CaseStatement();

	// Make a deep copy, and make the copy a derived object if needed.
virtual Statement* clone();

	// Accept a visitor to this RTL
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

	// Set and return the Exp representing the switch variable
	SWITCH_INFO* getSwitchInfo(); 
	void		setSwitchInfo(SWITCH_INFO* pss);
	
virtual void	print(std::ostream& os = std::cout);

	// Replace all instances of "search" with "replace".
virtual bool	searchAndReplace(Exp* search, Exp* replace);
	
	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.
virtual bool	searchAll(Exp* search, std::list<Exp*> &result);
	
#if 0
	// Used for type analysis. Stores type information that
	// can be gathered from the RTL instruction inside a
	// data structure within BBBlock inBlock
	void storeUseDefineStruct(BBBlock& inBlock);   
#endif	   

	// code generation
virtual void	generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);
	
	// dataflow analysis
virtual bool	usesExp(Exp *e);
protected:
virtual bool	doReplaceRef(Exp* from, Exp* to);
public:

	// simplify all the uses/defs in this RTL
virtual void	simplify();

virtual void	fromSSAform(igraph& ig);

		// Replace registers with locals
virtual	void	regReplace(UserProc* proc);

	friend class XMLProgParser;
};			// class CaseStatement

struct ReturnInfo {
		Exp*	e;
		Type*	type;
				ReturnInfo();
};

/*==============================================================================
 * CallStatement: represents a high level call. Information about parameters and
 * the like are stored here.
 *============================================================================*/
class CallStatement: public GotoStatement {
		bool		returnAfterCall;// True if call is effectively followed by a return.
	
		// The list of arguments passed by this call
		std::vector<Exp*> arguments;
		// The list of arguments implicitly passed as a result of the calling 
		// convention of the called procedure or the actual arguments
		std::vector<Exp*> implicitArguments;

		// The set of locations that are defined by this call, and their types
		// Note: returns is not a great name, these are really definitions. The opposite of the returns in a
		// ReturnStatement (which don't define anything, and hence don't store a type).
		std::vector<ReturnInfo> returns;

		// Destination of call
		Proc*		procDest;

public:
				CallStatement();
virtual			~CallStatement();

	// Make a deep copy, and make the copy a derived object if needed.
virtual Statement*	clone();

	// Accept a visitor to this stmt
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

		void	setArguments(std::vector<Exp*>& arguments);
		// Set implicit arguments: so far, for testing only:
		void	setImpArguments(std::vector<Exp*>& arguments);
		void	setReturns(std::vector<Exp*>& returns);// Set call's return locs
		void	setSigArguments();			// Set arguments based on signature
		std::vector<Exp*>& getArguments();		// Return call's arguments
		int		getNumReturns();
		Exp		*getReturnExp(int i);
		int		findReturn(Exp *e);
		void	removeReturn(Exp *e);
		void	ignoreReturn(Exp *e);
		void	ignoreReturn(int n);
		void	addReturn(Exp *e, Type* ty = NULL);
		std::vector<ReturnInfo>& getReturns() {return returns;}
		Exp		*getProven(Exp *e);
		// Substitute the various components of expression e with the appropriate actual arguments
		Exp		*substituteParams(Exp *e);
		void	addArgument(Exp *e);
		// Treat e as the expression for a parameter, and return the actual, or failing that, the implicit parameter
		Exp*	findArgument(Exp* e);
		Exp*	getArgumentExp(int i);
		Exp*	getImplicitArgumentExp(int i);
		std::vector<Exp*>& getImplicitArguments() {return implicitArguments;}
		int		getNumImplicitArguments() {return implicitArguments.size();}
		void	setArgumentExp(int i, Exp *e);
		void	setNumArguments(int i);
		int		getNumArguments();
		void	removeArgument(int i);
		void	removeImplicitArgument(int i);
		Type	*getArgumentType(int i);
		void	truncateArguments();
		void	clearLiveEntry();


virtual void	print(std::ostream& os = std::cout);

		// general search
virtual bool	search(Exp *search, Exp *&result);

		// Replace all instances of "search" with "replace".
virtual bool	searchAndReplace(Exp* search, Exp* replace);
	
		// Searches for all instances of a given subexpression within this
		// expression and adds them to a given list in reverse nesting order.
virtual bool	searchAll(Exp* search, std::list<Exp*> &result);

		// Set and return whether the call is effectively followed by a return.
		// E.g. on Sparc, whether there is a restore in the delay slot.
		void	setReturnAfterCall(bool b);
		bool	isReturnAfterCall();

		// Set and return the list of Exps that occur *after* the call (the
		// list of exps in the RTL occur before the call). Useful for odd patterns.
		void	setPostCallExpList(std::list<Exp*>* le);
		std::list<Exp*>* getPostCallExpList();

		// Set and return the destination proc.
		void	setDestProc(Proc* dest);
		Proc*	getDestProc();

		// Generate constraints
virtual void	genConstraints(LocationSet& cons);

		// Data flow based type analysis
		void	dfaTypeAnalysis(bool& ch);

		// Replace registers with locals
virtual	void	regReplace(UserProc* proc);

		// code generation
virtual void	generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

		// dataflow analysis
virtual bool	usesExp(Exp *e);

		// dataflow related functions
virtual bool	propagateToAll() { assert(false); return false;}

virtual bool	isDefinition();
virtual void	getDefinitions(LocationSet &defs);

virtual Exp*	getLeft() {return getReturnExp(0);}
virtual void	setLeftFor(Exp* forExp, Exp* newExp);
		// get how to replace this statement in a use
virtual Exp*	getRight() { return NULL; }

		// inline any constants in the statement
virtual void	processConstants(Prog *prog);

		// simplify all the uses/defs in this RTL
virtual void	simplify();

		void	setIgnoreReturnLoc(bool b);

		void	decompile();

virtual void	fromSSAform(igraph& ig);
		
		// Insert actual arguments to match formal parameters
		void	insertArguments(StatementSet& rs);

virtual	Type*	getTypeFor(Exp* e);				// Get the type defined by this Statement for this location
virtual void	setTypeFor(Exp* e, Type* ty);	// Set the type for this location, defined in this statement
		// Process this call for ellipsis parameters. If found, in a printf/scanf call, truncate the
		// number of parameters if needed, and return true if any signature parameters added
		bool	ellipsisProcessing(Prog* prog);

protected:
virtual bool	doReplaceRef(Exp* from, Exp* to);
		bool	convertToDirect();

		void	updateArgumentWithType(int n);
		void	updateReturnWithType(int n);
		void	appendArgument(Exp *e) { arguments.push_back(e); }
		void	appendImplicitArgument(Exp *e) { implicitArguments.push_back(e); }
	friend class XMLProgParser;
};		// class CallStatement


/*==============================================================================
 * ReturnStatement: represents a high level return.
 *============================================================================*/
class ReturnStatement: public GotoStatement {
protected:
		// number of bytes that this return pops
		int		nBytesPopped;

		// value returned. No types stored. To find the type of a return, call ascendType on the return expression
		std::vector<Exp*> returns;

		// Native address of the (only) return instruction
		// Needed for branching to this only return statement
		ADDRESS	retAddr;

public:
				ReturnStatement();
virtual			~ReturnStatement();

		// Make a deep copy, and make the copy a derived object if needed.
virtual Statement* clone();

		// Accept a visitor to this RTL
virtual bool	accept(StmtVisitor* visitor);
virtual bool	accept(StmtExpVisitor* visitor);
virtual bool	accept(StmtModifier* visitor);

		// print
virtual void	print(std::ostream& os = std::cout);

		// From SSA form
virtual void	fromSSAform(igraph& igm);

		// code generation
virtual void	generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

		// simplify all the uses/defs in this RTL
virtual void	simplify();

		// general search
virtual bool	search(Exp*, Exp*&);

		// Replace all instances of "search" with "replace".
virtual bool	searchAndReplace(Exp* search, Exp* replace);
	
		// Searches for all instances of a given subexpression within this
		// expression and adds them to a given list in reverse nesting order.	 
virtual bool	searchAll(Exp* search, std::list<Exp*> &result);

		// returns true if this statement uses the given expression
virtual bool	usesExp(Exp *e);

virtual bool	doReplaceRef(Exp* from, Exp* to);
		int		getNumBytesPopped() { return nBytesPopped; }
		void	setNumBytesPopped(int n) { nBytesPopped = n; }

		int		getNumReturns() { return returns.size(); }
		Exp		*getReturnExp(int n) { return returns[n]; }
		void	setReturnExp(int n, Exp *e) { returns[n] = e; }
		std::vector<Exp*>& getReturns() {return returns;}
		void	setSigArguments();	 // Set returns based on signature
		void	removeReturn(int n);
		void	addReturn(Exp *e);

		// Get and set the native address for the first and only return statement
		ADDRESS	getRetAddr() {return retAddr;}
		void	setRetAddr(ADDRESS r) {retAddr = r;}

virtual void	dfaTypeAnalysis(bool& ch);

		// Replace registers with locals
virtual	void	regReplace(UserProc* proc);

	friend class XMLProgParser;
};	// class ReturnStatement


#endif // __STATEMENT_H__
