#pragma once

/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       statement.h
 * OVERVIEW:   The Statement and related classes
 *  (Was dataflow.h a long time ago)
 ******************************************************************************/

/* Class hierarchy:   Statement@            (@ = abstract)
 *                  __/   |   \________________________
 *                 /      |            \               \
 *     GotoStatement  TypingStatement@  ReturnStatement JunctionStatement
 * BranchStatement_/     /          \
 * CaseStatement__/  Assignment@   ImpRefStatement
 * CallStatement_/  /   /    \ \________
 *       PhiAssign_/ Assign  BoolAssign \_ImplicitAssign
 */
#include "include/config.h"

#include "include/memo.h"
#include "db/exphelp.h" // For lessExpStar, lessAssignment etc
#include "include/types.h"
#include "include/managed.h"
#include "db/dataflow.h"  // For embedded objects DefCollector and UseCollector

#include <QtCore/QTextStream>

#include <vector>
#include <set>
#include <list>
#include <map>
#include <memory>
#include <cassert>

class BasicBlock;
class Prog;
class Function;
class UserProc;
class Exp;
class Const;
class RefExp;
class Cfg;
class Type;
class Instruction;
class Signature;
class StmtVisitor;
class StmtExpVisitor;
class StmtModifier;
class StmtPartModifier;
class HLLCode;
class Assign;
class RTL;
class InstructionSet;
class XMLProgParser;
class ReturnStatement;

typedef std::set<UserProc *>           CycleSet;
typedef std::shared_ptr<Exp>           SharedExp;
typedef std::unique_ptr<Instruction>   UniqInstruction;
typedef std::shared_ptr<Type>          SharedType;

/***************************************************************************/ /**
 * Kinds of Statements, or high-level register transfer lists.
 * changing the order of these will result in save files not working - trent
 ******************************************************************************/
enum STMT_KIND : uint8_t   // ' : uint8_t' so that it can be forward declared in rtl.h
{
	STMT_ASSIGN = 0,
	STMT_PHIASSIGN,
	STMT_IMPASSIGN,
	STMT_BOOLASSIGN, // For "setCC" instructions that set destination
	// to 1 or 0 depending on the condition codes.
	STMT_CALL,
	STMT_RET,
	STMT_BRANCH,
	STMT_GOTO,
	STMT_CASE, // Represent  a switch statement
	STMT_IMPREF,
	STMT_JUNCTION
};

/***************************************************************************/ /**
 * BRANCH_TYPE: These values indicate what kind of conditional jump or
 * conditonal assign is being performed.
 * Changing the order of these will result in save files not working - trent
 ******************************************************************************/
enum BRANCH_TYPE
{
	BRANCH_JE = 0, // Jump if equals
	BRANCH_JNE,    // Jump if not equals
	BRANCH_JSL,    // Jump if signed less
	BRANCH_JSLE,   // Jump if signed less or equal
	BRANCH_JSGE,   // Jump if signed greater or equal
	BRANCH_JSG,    // Jump if signed greater
	BRANCH_JUL,    // Jump if unsigned less
	BRANCH_JULE,   // Jump if unsigned less or equal
	BRANCH_JUGE,   // Jump if unsigned greater or equal
	BRANCH_JUG,    // Jump if unsigned greater
	BRANCH_JMI,    // Jump if result is minus
	BRANCH_JPOS,   // Jump if result is positive
	BRANCH_JOF,    // Jump if overflow
	BRANCH_JNOF,   // Jump if no overflow
	BRANCH_JPAR    // Jump if parity even (Intel only)
};

//    //    //    //    //    //    //    //    //    //    //    //    //    //
//
//    A b s t r a c t      C l a s s      S t a t e m e n t //
//
//    //    //    //    //    //    //    //    //    //    //    //    //    //

/* Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 */
class Instruction
{
	typedef std::map<SharedExp, int, lessExpStar> mExpInt;

	friend class XMLProgParser;
	
protected:
	BasicBlock *m_parent; // contains a pointer to the enclosing BB
	UserProc *m_proc;     // procedure containing this statement
	int m_number;         // Statement number for printing
	
#if USE_DOMINANCE_NUMS
	int m_dominanceNum;   // Like a statement number, but has dominance properties

public:
	int getDomNumber() const { return m_dominanceNum; }
	void setDomNumber(int dn) { m_dominanceNum = dn; }

protected:
#endif
	
	STMT_KIND m_kind; // Statement kind (e.g. STMT_BRANCH)
	unsigned int m_lexBegin, m_lexEnd;

public:
	Instruction()
		: m_parent(nullptr)
		, m_proc(nullptr)
		, m_number(0) {}
		
	virtual ~Instruction() {}

	/// get/set the enclosing BB, etc
	BasicBlock *getBB() { return m_parent; }
	const BasicBlock *getBB() const { return m_parent; }
	void setBB(BasicBlock *bb) { m_parent = bb; }

	// bool operator==(Statement& o);
	// Get and set *enclosing* proc (not destination proc)
	void setProc(UserProc *p);

	UserProc *getProc() const { return m_proc; }

	int getNumber() const { return m_number; }
	virtual void setNumber(int num) { m_number = num; } ///< Overridden for calls (and maybe later returns)

	STMT_KIND getKind() const { return m_kind; }
	void setKind(STMT_KIND k) { m_kind = k; }

	virtual Instruction *clone() const = 0;  ///< Make copy of self

	// Accept a visitor (of various kinds) to this Statement. Return true to continue visiting
	virtual bool accept(StmtVisitor *visitor)      = 0;
	virtual bool accept(StmtExpVisitor *visitor)   = 0;
	virtual bool accept(StmtModifier *visitor)     = 0;
	virtual bool accept(StmtPartModifier *visitor) = 0;

	void setLexBegin(unsigned int n) { m_lexBegin = n; }
	void setLexEnd(unsigned int n) { m_lexEnd = n; }
	unsigned int getLexBegin() const { return m_lexBegin; }
	unsigned int getLexEnd() const { return m_lexEnd; }

	/// returns true if this statement defines anything
	virtual bool isDefinition() const = 0;
	bool isNullStatement() const; ///< true if is a null statement

	virtual bool isTyping() const { return false; } // Return true if a TypingStatement
	
	/// true if this statement is a standard assign
	bool isAssign() const { return m_kind == STMT_ASSIGN; }
	/// true if this statement is a any kind of assignment
	bool isAssignment() const
	{
		return m_kind == STMT_ASSIGN || m_kind == STMT_PHIASSIGN || m_kind == STMT_IMPASSIGN || m_kind == STMT_BOOLASSIGN;
	}

	bool isPhi() const { return m_kind == STMT_PHIASSIGN; }      ///< true    if this statement is a phi assignment
	bool isImplicit() const { return m_kind == STMT_IMPASSIGN; } ///< true if this statement is an implicit assignment
	bool isFlagAssgn() const;                                    ///< true if this statment is a flags assignment

	bool isImpRef() const { return m_kind == STMT_IMPREF; } ///< true of this statement is an implicit reference

	virtual bool isGoto() { return m_kind == STMT_GOTO; }
	virtual bool isBranch() { return m_kind == STMT_BRANCH; }

	// true if this statement is a junction
	bool isJunction() const { return m_kind == STMT_JUNCTION; }

	/// true if this statement is a call
	bool isCall() const { return m_kind == STMT_CALL; }

	/// true if this statement is a BoolAssign
	bool isBool() const { return m_kind == STMT_BOOLASSIGN; }

	/// true if this statement is a ReturnStatement
	bool isReturn() const { return m_kind == STMT_RET; }

	/// true if this statement is a decoded ICT.
	/// \note for now, it only represents decoded indirect jump instructions
	bool isHL_ICT() const { return m_kind == STMT_CASE; }

	bool isCase() { return m_kind == STMT_CASE; }

	/// true if this is a fpush/fpop
	bool isFpush() const;
	bool isFpop() const;

	/// Classes with no definitions (e.g. GotoStatement and children) don't override this
	/// returns a set of locations defined by this statement in a LocationSet argument.
	virtual void getDefinitions(LocationSet& /*def*/) const {}

	// set the left for forExp to newExp

	virtual bool definesLoc(SharedExp /*loc*/) const { return false; }  // True if this Statement defines loc

	// returns true if this statement uses the given expression
	virtual bool usesExp(const Exp&) const = 0;

	// statements should be printable (for debugging)
	virtual void print(QTextStream& os, bool html = false) const = 0;

	// print functions
	
	void printAsUse(QTextStream& os) const { os << m_number; }
	void printAsUseBy(QTextStream& os) const { os << m_number; }
	void printNum(QTextStream& os) const { os << m_number; }
	char *prints() const; // For logging, was also for debugging
	
	// This version prints much better in gdb
	void dump() const;    // For debugging

	// general search
	virtual bool search(const Exp& search, SharedExp& result) const = 0;
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const = 0;

	/// general search and replace. Set cc true to change collectors as well. Return true if any change
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) = 0; // TODO: consider constness

	// True if can propagate to expression e in this Statement.
	// Return true if can propagate to Exp* e (must be a RefExp to return true)
	// Note: does not consider whether e is able to be renamed (from a memory Primitive point of view), only if the
	// definition can be propagated TO this stmt
	// Note: static member function
	static bool canPropagateToExp(Exp& e);
	
	/***************************************************************************/ /**
	* \brief Propagate to this statement
	* \param destCounts is a map that indicates how may times a statement's definition is used
	* \param convert set true if an indirect call is changed to direct (otherwise, no change)
	* \param force set to true to propagate even memofs (for switch analysis)
	* \param usedByDomPhi is a set of subscripted locations used in phi statements
	* \returns true if a change
	******************************************************************************/
	bool propagateTo(bool& convert, mExpInt *destCounts = nullptr, LocationSet *usedByDomPhi = nullptr,
					 bool force = false);
	
	/// Experimental: may want to propagate flags first,
	/// without tests about complexity or the propagation limiting heuristic
	bool propagateFlagsTo();

	// code generation
	virtual void generateCode(HLLCode *hll, BasicBlock *Parent, int indLevel) = 0;
	virtual void simplify() = 0; ///< simpify internal expressions

	/// simplify internal address expressions (a[m[x]] -> x) etc
	/// Only Assignments override at present
	virtual void simplifyAddr() {}

	/// map registers and temporaries to local variables
	void mapRegistersToLocals();

	/// The last part of the fromSSA logic: replace subscripted locations with suitable local variables
	void replaceSubscriptsWithLocals();

	/// insert casts where needed, since fromSSA will erase type information
	void insertCasts();

	// fixSuccessor
	// Only Assign overrides at present
	virtual void fixSuccessor() {}

	/// Generate constraints (for constraint based type analysis)
	virtual void genConstraints(LocationSet& /*cons*/) {}

	// Data flow based type analysis
	virtual void dfaTypeAnalysis(bool& /*ch*/) {}  // Use the type information in this Statement
	SharedType meetWithFor(SharedType ty, SharedExp e, bool& ch); // Meet the type associated with e with ty

public:
	// helper functions
	bool isFirstStatementInBB() const;
	bool isLastStatementInBB() const;
	Instruction *getNextStatementInBB() const;
	Instruction *getPreviousStatementInBB() const;

	//    //    //    //    //    //    //    //    //    //
	//                                                    //
	//    Statement visitation functions                  //
	//                                                    //
	//    //    //    //    //    //    //    //    //    //

	/// Find the locations used by expressions in this Statement.
	/// Use the StmtExpVisitor and UsedLocsFinder visitor classes
	/// Adds (inserts) all locations (registers or memory etc) used by this statement
	/// Set \a cc to true to count the uses in collectors
	/// \param used set of used locations
	/// \param cc count collectors
	/// \param memOnly - only add memory references.
	void addUsedLocs(LocationSet& used, bool cc = false, bool memOnly = false);
	
	/// Special version of Statement::addUsedLocs for finding used locations.
	/// \return true if defineAll was found
	bool addUsedLocals(LocationSet& used);
	
	/// Fix references to the returns of call statements
	/// Bypass calls for references in this statement
	void bypass();
	
	/// replace a use of def->getLeft() by def->getRight() in this statement
	/// replaces a use in this statement with an expression from an ordinary assignment
	/// \returns true if change
	/// \note Internal use only
	bool replaceRef(SharedExp e, Assignment *def, bool& convert);
	
	/// Find all constants in this statement
	void findConstants(std::list<std::shared_ptr<Const> >& lc);
	
	/// Set or clear the constant subscripts (using a visitor)
	int setConscripts(int n);
	void clearConscripts();
	
	/// Strip all size casts
	void stripSizes();
	
	/// For all expressions in this Statement, replace any e with e{def}
	void subscriptVar(SharedExp e, Instruction *def /*, Cfg* cfg */);

	// Cast the constant num to type ty. If a change was made, return true
	// Cast the constant num to be of type ty. Return true if a change made
	bool castConst(int num, SharedType ty);

	// Map expressions to locals
	void dfaMapLocals();

	// End Statement visitation functions

	/// Get the type for the definition, if any, for expression e in this statement
	/// Overridden only by Assignment and CallStatement, and ReturnStatement.
	virtual SharedType getTypeFor(SharedExp) const { return nullptr; }
	/// Set the type for the definition of e in this Statement
	virtual void setTypeFor(SharedExp, SharedType) { assert(false); }

	/// Parameter convert is set true if an indirect call is converted to direct
	/// Return true if a change made
	/// Note: this procedure does not control what part of this statement is propagated to
	/// Propagate to e from definition statement def.
	/// Set convert to true if convert a call from indirect to direct.
	bool doPropagateTo(SharedExp e, Assignment *def, bool& convert);

	/// returns true if e1 may alias e2
	bool calcMayAlias(SharedExp e1, SharedExp e2, int size) const;
	bool mayAlias(SharedExp e1, SharedExp e2, int size) const;
};


/// Print the Statement (etc) pointed to by p
QTextStream& operator<<(QTextStream& os, const Instruction *p);
QTextStream& operator<<(QTextStream& os, const InstructionSet *p);
QTextStream& operator<<(QTextStream& os, const LocationSet *p);


/***************************************************************************/ /**
 * TypingStatement is an abstract subclass of Statement. It has a type, representing the type of a reference or an
 * assignment
 ****************************************************************************/
class TypingStatement : public Instruction
{
protected:
	SharedType m_type; ///< The type for this assignment or reference

public:
	TypingStatement(SharedType ty); ///< Constructor

	// Get and set the type.
	SharedType getType() { return m_type; }
	const SharedType& getType() const { return m_type; }
	void setType(SharedType ty) { m_type = ty; }

	virtual bool isTyping() const override { return true; }
};

/***************************************************************************/ /**
 * Assignment is an abstract subclass of TypingStatement, holding a location
 ****************************************************************************/
class Assignment : public TypingStatement
{
protected:
	SharedExp lhs; // The left hand side

public:
	// Constructor, subexpression
	Assignment(SharedExp lhs);
	// Constructor, type, and subexpression
	Assignment(SharedType ty, SharedExp lhs);
	// Destructor
	virtual ~Assignment();

	// Clone
	virtual Instruction *clone() const override = 0;

	// We also want operator< for assignments. For example, we want ReturnStatement to contain a set of (pointers
	// to) Assignments, so we can automatically make sure that existing assignments are not duplicated
	// Assume that we won't want sets of assignments differing by anything other than LHSs
	bool operator<(const Assignment& o) { return lhs < o.lhs; }

	// Accept a visitor to this Statement
	virtual bool accept(StmtVisitor *visitor)      override = 0;
	virtual bool accept(StmtExpVisitor *visitor)   override = 0;
	virtual bool accept(StmtModifier *visitor)     override = 0;
	virtual bool accept(StmtPartModifier *visitor) override = 0;

	virtual void print(QTextStream& os, bool html = false) const override;
	virtual void printCompact(QTextStream& os, bool html = false) const = 0; // Without statement number

	virtual SharedType getTypeFor(SharedExp e) const override;    ///< Get the type for this assignment. It should define e
	virtual void setTypeFor(SharedExp e, SharedType ty) override; ///< Set the type for this assignment. It should define e

	virtual bool usesExp(const Exp& e) const override;               // PhiAssign and ImplicitAssign don't override

	virtual bool isDefinition() const override { return true; }
	virtual void getDefinitions(LocationSet& defs) const override;
	virtual bool definesLoc(SharedExp loc) const override; // True if this Statement defines loc

	// get how to access this lvalue
	virtual SharedExp getLeft() { return lhs; } // Note: now only defined for Assignments, not all Statements
	virtual const SharedExp& getLeft() const { return lhs; }

	virtual SharedExp getRight() const = 0;

	// set the lhs to something new
	void setLeft(SharedExp e) { lhs = e; }

	// memory depth
	int getMemDepth();

	// general search
	virtual bool search(const Exp& search, SharedExp& result) const override = 0;
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override = 0;

	// general search and replace
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override = 0;

	void generateCode(HLLCode *, BasicBlock *, int /*indLevel*/) override {}

	// simpify internal expressions
	virtual void simplify() override = 0;

	// simplify address expressions
	virtual void simplifyAddr() override;

	// generate Constraints
	virtual void genConstraints(LocationSet& cons) override;

	// Data flow based type analysis
	void dfaTypeAnalysis(bool& ch) override;

	friend class XMLProgParser;
}; // class Assignment

/***************************************************************************/ /**
 * Assign an ordinary assignment with left and right sides
 ***************************************************************************/
class Assign : public Assignment
{
	SharedExp rhs;
	SharedExp guard;

public:
	// Constructor, subexpressions
	Assign(SharedExp lhs, SharedExp r, SharedExp guard = nullptr);
	// Constructor, type and subexpressions
	Assign(SharedType ty, SharedExp lhs, SharedExp r, SharedExp guard = nullptr);
	// Default constructor, for XML parser
	Assign()
		: Assignment(nullptr)
		, rhs(nullptr)
		, guard(nullptr) {}
	// Copy constructor
	Assign(Assign& o);
	// Destructor
	~Assign() {}

	// Clone
	virtual Instruction *clone() const override;

	// get how to replace this statement in a use
	virtual SharedExp getRight() const override { return rhs; }
	SharedExp& getRightRef() { return rhs; }

	// set the rhs to something new
	void setRight(SharedExp e) { rhs = e; }

	// Accept a visitor to this Statement
	// Visiting from class StmtExpVisitor
	// Visit all the various expressions in a statement
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	
	// Visiting from class StmtModifier
	// Modify all the various expressions in a statement
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	virtual void printCompact(QTextStream& os, bool html = false) const override; // Without statement number

	// Guard
	void setGuard(SharedExp g) { guard = g; }
	SharedExp getGuard() { return guard; }
	bool isGuarded() { return guard != nullptr; }

	virtual bool usesExp(const Exp& e) const override;

	virtual bool isDefinition() const override { return true; }

	// general search
	virtual bool search(const Exp& search, SharedExp& result) const override;
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// general search and replace
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// memory depth
	int getMemDepth();

	// Generate code
	virtual void generateCode(HLLCode *hll, BasicBlock *Parent, int indLevel) override;

	// simpify internal expressions
	virtual void simplify() override;

	// simplify address expressions
	virtual void simplifyAddr() override;

	// fixSuccessor (succ(r2) -> r3)
	virtual void fixSuccessor() override;

	// generate Constraints
	virtual void genConstraints(LocationSet& cons) override;

	// Data flow based type analysis
	void dfaTypeAnalysis(bool& ch) override;

	// FIXME: I suspect that this was only used by adhoc TA, and can be deleted
	bool match(const char *pattern, std::map<QString, SharedExp>& bindings);

	friend class XMLProgParser;
}; // class Assign


// The below could almost be a RefExp. But we could not at one stage #include exp.h as part of statement,h; that's since
// changed so it is now possible, and arguably desirable.  However, it's convenient to have these members public
struct PhiInfo
{
	// A default constructor is required because CFG changes (?) can cause access to elements of the vector that
	// are beyond the current end, creating gaps which have to be initialised to zeroes so that they can be skipped
	PhiInfo() {} // : def(0), e(0) not initializing to help valgrind find locations of unset vals
	SharedExp         e; // The expression for the thing being defined (never subscripted)
	void              def(Instruction *def) { m_def = def; /*assert(def);*/ }
	Instruction       *def() { return m_def; }
	const Instruction *def() const { return m_def; }

protected:
	Instruction       *m_def; // The defining statement
};

/***************************************************************************/ /**
 * PhiAssign is a subclass of Assignment, having a left hand side, and a StatementVec with the references.
 * \code
 * m[1000] := phi{3 7 10}    // m[1000] is defined at statements 3, 7, and 10
 * m[r28{3}+4] := phi{2 8}   // the memof is defined at 2 and 8, and the r28 is defined at 3.
 * \endcode
 * The integers are really pointers to statements,printed as the statement number for compactness
 *
 * \note Although the left hand side is nearly always redundant, it is essential in at least one circumstance: when
 * finding locations used by some statement, and the reference is to a CallStatement returning multiple locations.
 ******************************************************************************/
class PhiAssign : public Assignment
{
	friend class XMLProgParser;
	
public:
	typedef std::map<BasicBlock *, PhiInfo> Definitions;
	typedef Definitions::iterator           iterator;
	typedef Definitions::const_iterator     const_iterator;

private:
	Definitions DefVec; // A vector of information about definitions

public:
	PhiAssign(SharedExp _lhs)
		: Assignment(_lhs) { m_kind = STMT_PHIASSIGN; }
	PhiAssign(SharedType ty, SharedExp _lhs)
		: Assignment(ty, _lhs) { m_kind = STMT_PHIASSIGN; }
	// Copy constructor (not currently used or implemented)
	PhiAssign(Assign& o);
	virtual ~PhiAssign() {}

	// Clone
	virtual Instruction *clone() const override;

	// get how to replace this statement in a use
	virtual SharedExp getRight() const override { return nullptr; }

	// Accept a visitor to this Statement
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	
	// Visiting from class StmtPartModifier
	// Modify all the various expressions in a statement, except for the top level of the LHS of assignments
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	virtual void printCompact(QTextStream& os, bool html = false) const override;

	// general search
	virtual bool search(const Exp& search, SharedExp& result) const override;
	
	/// FIXME: is this the right semantics for searching a phi statement, disregarding the RHS?
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// general search and replace
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	// Generate constraints
	virtual void genConstraints(LocationSet& cons) override;

	// Data flow based type analysis
	void dfaTypeAnalysis(bool& ch) override;

	//
	//    Phi specific functions
	//

	// Get or put the statement at index idx
	Instruction *getStmtAt(BasicBlock *idx)
	{
		if (DefVec.find(idx) == DefVec.end()) {
			return nullptr;
		}

		return DefVec[idx].def();
	}

	PhiInfo& getAt(BasicBlock *idx);
	void putAt(BasicBlock *idx, Instruction *d, SharedExp e);
	void simplifyRefs();

	virtual size_t getNumDefs() const { return DefVec.size(); }
	Definitions& getDefs() { return DefVec; }
	
	// A hack. Check MVE
	bool hasGlobalFuncParam();

	PhiInfo& front() { return DefVec.begin()->second; }
	PhiInfo& back() { return DefVec.rbegin()->second; }
	iterator begin() { return DefVec.begin(); }
	iterator end() { return DefVec.end(); }
	const_iterator cbegin() const { return DefVec.begin(); }
	const_iterator cend() const { return DefVec.end(); }
	iterator erase(iterator it) { return DefVec.erase(it); }

	// Convert this phi assignment to an ordinary assignment
	
	/// Convert this PhiAssignment to an ordinary Assignment. 
	/// Hopefully, this is the only place that Statements change from
	/// one class to another.  All throughout the code, we assume that the addresses of Statement objects do not change,
	/// so we need this slight hack to overwrite one object with another
	void convertToAssign(SharedExp rhs);

	// Generate a list of references for the parameters
	void enumerateParams(std::list<SharedExp>& le);
};

// An implicit assignment has only a left hand side. It is a placeholder for storing the types of parameters and
// globals.  That way, you can always find the type of a subscripted variable by looking in its defining Assignment
class ImplicitAssign : public Assignment
{
public:
	// Implicit Assignment
	/// Constructor and subexpression
	ImplicitAssign(SharedExp lhs);
	/// Constructor, type, and subexpression
	ImplicitAssign(SharedType ty, SharedExp lhs);
	ImplicitAssign(ImplicitAssign& o);
	
	// The first virtual function (here the destructor) can't be in statement.h file for gcc
	virtual ~ImplicitAssign();

	virtual Instruction *clone() const override;
	void dfaTypeAnalysis(bool& ch) override;

	// general search
	virtual bool search(const Exp& search, SharedExp& result) const override;
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// general search and replace
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	virtual void printCompact(QTextStream& os, bool html = false) const override;

	// Statement and Assignment functions
	virtual SharedExp getRight() const override { return nullptr; }
	virtual void simplify() override {}

	// Visitation
	// visit this Statement
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;
}; // class ImplicitAssign

/***************************************************************************/ /**
 * BoolAssign represents "setCC" type instructions, where some destination is set
 * (to 1 or 0) depending on the condition codes.
 * It has a condition Exp, similar to the BranchStatement class.
 * *==========================================================================*/
class BoolAssign : public Assignment
{
	BRANCH_TYPE jtCond; // the condition for setting true
	SharedExp pCond;    // Exp representation of the high level
	// condition: e.g. r[8] == 5
	bool bFloat;        // True if condition uses floating point CC
	int Size;           // The size of the dest

public:
	/***************************************************************************/ /**
	* \fn         BoolAssign::BoolAssign
	* \brief         Constructor.
	* \param         size - the size of the assignment
	******************************************************************************/
	BoolAssign(int size);
	virtual ~BoolAssign();

	// Make a deep copy, and make the copy a derived object if needed.
	/***************************************************************************/ /**
	* \fn        BoolAssign::clone
	* \brief     Deep copy clone
	* \returns   Pointer to a new Statement, a clone of this BoolAssign
	******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this Statement
	/// visit this Statement
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	// Set and return the BRANCH_TYPE of this scond as well as whether the
	// floating point condition codes are used.
	
	/***************************************************************************/ /**
	* \brief Sets the BRANCH_TYPE of this jcond as well as the flag
	* indicating whether or not the floating point condition codes
	* are used.
	* \param cond - the BRANCH_TYPE
	* \param usesFloat - this condional jump checks the floating point condition codes
	******************************************************************************/
	void setCondType(BRANCH_TYPE cond, bool usesFloat = false);

	BRANCH_TYPE getCond() const { return jtCond; }
	bool isFloat() const { return bFloat; }
	void setFloat(bool b) { bFloat = b; }

	// Set and return the Exp representing the HL condition
	
	/***************************************************************************/ /**
	* \brief Return the Exp expression containing the HL condition.
	* \returns Exp instance
	******************************************************************************/
	SharedExp getCondExpr() const;
	
	/***************************************************************************/ /**
	* \brief Set the Exp expression containing the HL condition.
	* \param pss Pointer to semantic string to set
	******************************************************************************/
	void setCondExpr(SharedExp pss);

	// As above, no delete (for subscripting)
	void setCondExprND(SharedExp e) { pCond = e; }
	int getSize() const { return Size; } // Return the size of the assignment
	
	/***************************************************************************/ /**
	* \brief Change this from an unsigned to a signed branch
	* \note Not sure if this is ever going to be used
	******************************************************************************/
	void makeSigned();

	/***************************************************************************/ /**
	* \fn    BoolAssign::printCompact
	* \brief Write a text representation to the given stream
	* \param os: stream
	* \param html - produce html encoded representation
	******************************************************************************/
	virtual void printCompact(QTextStream& os, bool html = false) const override;
	
	/// code generation
	virtual void generateCode(HLLCode *hll, BasicBlock *, int indLevel) override;
	
	/// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	// Statement functions
	virtual bool isDefinition() const override { return true; }
	
	// All the Assignment-derived classes have the same definitions: the lhs
	virtual void getDefinitions(LocationSet& def) const override;

	virtual SharedExp getRight() const override { return getCondExpr(); }
	
	virtual bool usesExp(const Exp& e) const override;
	virtual bool search(const Exp& search, SharedExp& result) const override;
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// a hack for the SETS macro
	// This is for setting up SETcc instructions; see include/decoder.h macro SETS
	void setLeftFromList(std::list<Instruction *> *stmts);

	virtual void dfaTypeAnalysis(bool& ch) override;

	friend class XMLProgParser;
}; // class BoolAssign

// An implicit reference has only an expression. It holds the type information that results from taking the address
// of a location. Note that dataflow can't decide which local variable (in the decompiled output) is being taken,
// if there is more than one local variable sharing the same memory address (separated then by type).
class ImpRefStatement : public TypingStatement
{
	SharedExp addressExp; // The expression representing the address of the location referenced

public:
	// Constructor, subexpression
	ImpRefStatement(SharedType ty, SharedExp a)
		: TypingStatement(ty)
		, addressExp(a) { m_kind = STMT_IMPREF; }
	SharedExp getAddressExp() { return addressExp; }
	SharedType getType() { return m_type; }
	void meetWith(SharedType ty, bool& ch); // Meet the internal type with ty. Set ch if a change

	// Virtuals
	virtual Instruction *clone() const override;
	virtual bool accept(StmtVisitor *) override;
	virtual bool accept(StmtExpVisitor *) override;
	virtual bool accept(StmtModifier *) override;
	virtual bool accept(StmtPartModifier *) override;

	virtual bool isDefinition() const override { return false; }
	virtual bool usesExp(const Exp&) const override { return false; }
	virtual bool search(const Exp&, SharedExp&) const override;
	virtual bool searchAll(const Exp&, std::list<SharedExp, std::allocator<SharedExp> >&) const override;

	virtual bool searchAndReplace(const Exp&, SharedExp, bool cc = false) override;
	virtual void generateCode(HLLCode *, BasicBlock *, int)  override {}
	virtual void simplify() override;
	
	// NOTE: ImpRefStatement not yet used
	virtual void print(QTextStream& os, bool html = false) const override;
}; // class ImpRefStatement

/*=============================================================================
 * GotoStatement has just one member variable, an expression representing the
 * jump's destination (an integer constant for direct jumps; an expression
 * for register jumps). An instance of this class will never represent a
 * return or computed call as these are distinguised by the decoder and are
 * instantiated as CallStatements and ReturnStatements respecitvely.
 * This class also represents unconditional jumps with a fixed offset
 * (e.g BN, Ba on SPARC).
 *===========================================================================*/
class GotoStatement : public Instruction
{
protected:
	SharedExp pDest;   ///< Destination of a jump or call. This is the absolute destination for both
	// static and dynamic CTIs.
	bool m_isComputed; ///< True if this is a CTI with a computed destination address.
	// NOTE: This should be removed, once CaseStatement and HLNwayCall are implemented
	// properly.
	std::shared_ptr<Const> constDest() { return std::static_pointer_cast<Const>(pDest); }
	const std::shared_ptr<const Const> constDest() const { return std::static_pointer_cast<const Const>(pDest); }

public:
	GotoStatement();
	
	/***************************************************************************/ /**
	* \brief        Construct a jump to a fixed address
	* \param        uDest native address of destination
	******************************************************************************/
	GotoStatement(ADDRESS jumpDest);
	
	/***************************************************************************/ /**
	* \fn        GotoStatement::~GotoStatement
	* \brief        Destructor
	******************************************************************************/
	virtual ~GotoStatement();

	/***************************************************************************/ /**
	* \fn        GotoStatement::clone
	* \brief     Deep copy clone
	* \returns   Pointer to a new Statement, a clone of this GotoStatement
	******************************************************************************/
	virtual Instruction *clone() const override;  ///< Make a deep copy, and make the copy a derived object if needed.

	// Accept a visitor to this Statement
	// visit this Statement in the RTL
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	// Set and return the destination of the jump. The destination is either an Exp, or an ADDRESS that
	// is converted to a Exp.
	/***************************************************************************/ /**
	* \brief        Set the destination of this jump to be a given fixed address.
	* \param   addr - the new fixed address
	******************************************************************************/
	void setDest(SharedExp pd);
	void setDest(ADDRESS addr);
	
	/***************************************************************************/ /**
	* \brief        Returns the destination of this CTI.
	* \returns Pointer to the Exp representing the dest of this jump
	******************************************************************************/
	virtual SharedExp getDest();
	virtual const SharedExp getDest() const;

	/***************************************************************************/ /**
	* \brief Get the fixed destination of this CTI. Assumes destination
	*        simplication has already been done so that a fixed dest will
	*        be of the Exp form:
	*        opIntConst dest
	* \returns Fixed dest or NO_ADDRESS if there isn't one, For dynamic CTIs,
	*          returns NO_ADDRESS.
	******************************************************************************/
	ADDRESS getFixedDest() const;
	
	/***************************************************************************/ /**
	* \brief        Adjust the destination of this CTI by a given amount. Causes
	*                    an error is this destination is not a fixed destination
	*                    (i.e. a constant offset).
	* \param   delta - the amount to add to the destination (can be
	*                  negative)
	******************************************************************************/
	void adjustFixedDest(int delta);

	/***************************************************************************/ /**
	* \fn      GotoStatement::setIsComputed
	* \brief      Sets the fact that this call is computed.
	* \note This should really be removed, once CaseStatement and
	*                    HLNwayCall are implemented properly
	******************************************************************************/
	void setIsComputed(bool b = true);
	
	/***************************************************************************/ /**
	* \fn      GotoStatement::isComputed
	* \brief      Returns whether or not this call is computed.
	* \note          This should really be removed, once CaseStatement and HLNwayCall
	*                    are implemented properly
	* \returns           this call is computed
	******************************************************************************/
	bool isComputed() const;

	/***************************************************************************/ /**
	* \fn    GotoStatement::print
	* \brief Display a text reprentation of this RTL to the given stream
	* \note  Usually called from RTL::print, in which case the first 9
	*        chars of the print have already been output to os
	* \param os - stream to write to
	* \param html - produce html encoded representation
	******************************************************************************/
	virtual void print(QTextStream& os, bool html = false) const override;

	// general search
	virtual bool search(const Exp&, SharedExp&) const override;

	// Replace all instances of "search" with "replace".
	/***************************************************************************/ /**
	* \fn        GotoStatement::searchAndReplace
	* \brief        Replace all instances of search with replace.
	* \param search - a location to search for
	* \param replace - the expression with which to replace it
	* \param cc - ignored
	* \returns True if any change
	******************************************************************************/
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;


	/***************************************************************************/ /**
	* \fn        GotoStatement::searchAll
	* \brief        Find all instances of the search expression
	* Searches for all instances of a given subexpression within this
	* expression and adds them to a given list in reverse nesting order.
	* 
	* \param search - a location to search for
	* \param result - a list which will have any matching exprs
	*                 appended to it
	* \returns true if there were any matches
	******************************************************************************/
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// code generation
	virtual void generateCode(HLLCode *, BasicBlock *, int) override;

	// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	// Statement virtual functions
	virtual bool isDefinition() const override { return false; }
	virtual bool usesExp(const Exp&) const override;

	friend class XMLProgParser;
}; // class GotoStatement

class JunctionStatement : public Instruction
{
public:
	JunctionStatement() { m_kind = STMT_JUNCTION; }

	virtual Instruction *clone() const override { return new JunctionStatement(); }

	// Accept a visitor (of various kinds) to this Statement. Return true to continue visiting
	bool accept(StmtVisitor *visitor) override;
	bool accept(StmtExpVisitor *visitor) override;
	bool accept(StmtModifier *visitor) override;
	bool accept(StmtPartModifier *visitor) override;

	// returns true if this statement defines anything
	bool isDefinition() const override { return false; }

	bool usesExp(const Exp&) const override { return false; }

	void print(QTextStream& os, bool html = false) const override;

	// general search
	bool search(const Exp& /*search*/, SharedExp& /*result*/) const override { return false; }
	bool searchAll(const Exp& /*search*/, std::list<SharedExp>& /*result*/) const override { return false; }

	/// general search and replace. Set cc true to change collectors as well. Return true if any change
	bool searchAndReplace(const Exp& /*search*/, SharedExp /*replace*/, bool /*cc*/ = false)  override { return false; }

	void generateCode(HLLCode * /*hll*/, BasicBlock * /*pbb*/, int /*indLevel*/)  override {}

	// simpify internal expressions
	void simplify() override {}

	bool isLoopJunction() const;
};

/***************************************************************************/ /**==
 * BranchStatement has a condition Exp in addition to the destination of the jump.
 *==============================================================================*/
class BranchStatement : public GotoStatement
{
	BRANCH_TYPE jtCond; // The condition for jumping
	SharedExp pCond;    // The Exp representation of the high level condition: e.g., r[8] == 5
	bool bFloat;        // True if uses floating point CC
	// jtCond seems to be mainly needed for the Pentium weirdness.
	// Perhaps bFloat, jtCond, and size could one day be merged into a type
	int size;         // Size of the operands, in bits

public:
	/***************************************************************************/ /**
	* \fn        BranchStatement::BranchStatement
	* \brief        Constructor.
	******************************************************************************/
	BranchStatement();
	
	/***************************************************************************/ /**
	* \fn        BranchStatement::~BranchStatement
	* \brief        Destructor
	******************************************************************************/
	virtual ~BranchStatement();

	// Make a deep copy, and make the copy a derived object if needed.
	/***************************************************************************/ /**
	* \fn        BranchStatement::clone
	* \brief        Deep copy clone
	* \returns             Pointer to a new Instruction, a clone of this BranchStatement
	******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this Statement
	// visit this stmt
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	// Set and return the BRANCH_TYPE of this jcond as well as whether the
	// floating point condition codes are used.
	/***************************************************************************/ /**
	* \fn    BranchStatement::setCondType
	* \brief Sets the BRANCH_TYPE of this jcond as well as the flag
	*        indicating whether or not the floating point condition codes
	*        are used.
	* \param cond - the BRANCH_TYPE
	* \param usesFloat - this condional jump checks the floating point condition codes
	******************************************************************************/
	void setCondType(BRANCH_TYPE cond, bool usesFloat = false);

	BRANCH_TYPE getCond() { return jtCond; }
	bool isFloat() { return bFloat; }
	void setFloat(bool b) { bFloat = b; }

	// Set and return the Exp representing the HL condition
	/***************************************************************************/ /**
	* \fn      BranchStatement::getCondExpr
	* \brief   Return the SemStr expression containing the HL condition.
	* \returns ptr to an expression
	******************************************************************************/
	SharedExp getCondExpr() const;
	
	/***************************************************************************/ /**
	* \fn          BranchStatement::setCondExpr
	* \brief       Set the SemStr expression containing the HL condition.
	* \param       pe - Pointer to Exp to set
	******************************************************************************/
	void setCondExpr(SharedExp pe);

	BasicBlock *getFallBB();
	BasicBlock *getTakenBB();
	
	/// not that if you set the taken BB or fixed dest first,
	/// you will not be able to set the fall BB
	void setFallBB(BasicBlock *bb);
	void setTakenBB(BasicBlock *bb);

	// Probably only used in front386.cc: convert this from an unsigned to a
	// signed conditional branch
	/***************************************************************************/ /**
	* \fn        BranchStatement::makeSigned
	* \brief        Change this from an unsigned to a signed branch
	******************************************************************************/
	void makeSigned();

	/***************************************************************************/ /**
	* \fn        BranchStatement::print
	* \brief        Write a text representation to the given stream
	* \param        os: stream
	* \param html - produce html encoded representation
	******************************************************************************/
	void print(QTextStream& os, bool html = false) const override;

	// general search
	bool search(const Exp& search, SharedExp& result) const override;

	/***************************************************************************/ /**
	* \fn    BranchStatement::searchAndReplace
	* \brief Replace all instances of search with replace.
	* \param search - a location to search for
	* \param replace - the expression with which to replace it
	* \param cc - ignored
	* \returns True if any change
	******************************************************************************/
	bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.
	
	/***************************************************************************/ /**
	* \brief   Find all instances of the search expression
	* \param   search - a location to search for
	* \param   result - a list which will have any matching exprs
	*          appended to it
	* \returns true if there were any matches
	******************************************************************************/
	bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// code generation
	void generateCode(HLLCode *, BasicBlock *, int) override;

	// dataflow analysis
	bool usesExp(const Exp& e) const override;

	// simplify all the uses/defs in this Statememt
	void simplify() override;

	// Generate constraints
	void genConstraints(LocationSet& cons) override;

	// Data flow based type analysis
	void dfaTypeAnalysis(bool& ch) override;

	friend class XMLProgParser;
}; // class BranchStatement

/***************************************************************************/ /**
 * CaseStatement is derived from GotoStatement. In addition to the destination
 * of the jump, it has a switch variable Exp.
 ******************************************************************************/
struct SWITCH_INFO
{
	SharedExp pSwitchVar;  // Ptr to Exp repres switch var, e.g. v[7]
	char      chForm;      // Switch form: 'A', 'O', 'R', 'H', or 'F' etc
	int       iLower;      // Lower bound of the switch variable
	int       iUpper;      // Upper bound for the switch variable
	ADDRESS   uTable;      // Native address of the table, or ptr to array of values for form F
	int       iNumTable;   // Number of entries in the table (form H only)
	int       iOffset = 0; // Distance from jump to table (form R only)
	// int        delta;            // Host address - Native address
};

class CaseStatement : public GotoStatement
{
	SWITCH_INFO *pSwitchInfo; // Ptr to struct with info about the switch

public:
	/***************************************************************************/ /**
	* \fn        CaseStatement::CaseStatement
	* \brief        Constructor.
	******************************************************************************/
	CaseStatement();
	
	/***************************************************************************/ /**
	* \fn    CaseStatement::~CaseStatement
	* \brief Destructor
	* \note  Don't delete the pSwitchVar; it's always a copy of something else (so don't delete twice)
	******************************************************************************/
	virtual ~CaseStatement();

	// Make a deep copy, and make the copy a derived object if needed.
	/***************************************************************************/ /**
	* \fn      CaseStatement::clone
	* \brief   Deep copy clone
	* \returns Pointer to a new Instruction that is a clone of this one
	******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this Statememt
	// visit this stmt
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	// Set and return the Exp representing the switch variable
	/***************************************************************************/ /**
	* \fn      CaseStatement::getSwitchInfo
	* \brief   Return a pointer to a struct with switch information in it
	* \returns SWITCH_INFO struct
	******************************************************************************/
	
	SWITCH_INFO *getSwitchInfo();
	

	/***************************************************************************/ /**
	* \fn    CaseStatement::setSwitchInfo
	* \brief Set a pointer to a SWITCH_INFO struct
	* \param psi Pointer to SWITCH_INFO struct
	******************************************************************************/
	void setSwitchInfo(SWITCH_INFO *psi);

	/***************************************************************************/ /**
	* \fn    CaseStatement::print
	* \brief Write a text representation to the given stream
	* \param os - target stream
	* \param html - produce html encoded representation
	******************************************************************************/
	virtual void print(QTextStream& os, bool html = false) const override;

	// Replace all instances of "search" with "replace".
	/***************************************************************************/ /**
	* \fn    CaseStatement::searchAndReplace
	* \brief Replace all instances of search with replace.
	* \param search - a location to search for
	* \param replace - the expression with which to replace it
	* \param cc - ignored
	* \returns             True if any change
	******************************************************************************/
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.
	
	/***************************************************************************/ /**
	* \fn    CaseStatement::searchAll
	* \brief Find all instances of the search expression
	* \param search - a location to search for
	* \param result - a list which will have any matching exprs appended to it
	* \returns true if there were any matches
	******************************************************************************/
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// code generation
	virtual void generateCode(HLLCode *, BasicBlock *, int) override;

	// dataflow analysis
	virtual bool usesExp(const Exp& e) const override;

public:
	// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	friend class XMLProgParser;
}; // class CaseStatement

/***************************************************************************/ /**
 * CallStatement: represents a high level call. Information about parameters and the like are stored here.
 ******************************************************************************/
class CallStatement : public GotoStatement
{
	bool returnAfterCall; // True if call is effectively followed by a return.

	// The list of arguments passed by this call, actually a list of Assign statements (location := expr)
	StatementList arguments;

	// The list of defines for this call, a list of ImplicitAssigns (used to be called returns).
	// Essentially a localised copy of the modifies of the callee, so the callee could be deleted. Stores types and
	// locations.  Note that not necessarily all of the defines end up being declared as results.
	StatementList defines;

	// Destination of call. In the case of an analysed indirect call, this will be ONE target's return statement.
	// For an unanalysed indirect call, or a call whose callee is not yet sufficiently decompiled due to recursion,
	// this will be nullptr
	Function *procDest;

	// The signature for this call. NOTE: this used to be stored in the Proc, but this does not make sense when
	// the proc happens to have varargs
	std::shared_ptr<Signature> signature;

	// A UseCollector object to collect the live variables at this call. Used as part of the calculation of
	// results
	UseCollector useCol;

	// A DefCollector object to collect the reaching definitions; used for bypassAndPropagate/localiseExp etc; also
	// the basis for arguments if this is an unanlysed indirect call
	DefCollector defCol;

	// Pointer to the callee ReturnStatement. If the callee is unanlysed, this will be a special ReturnStatement
	// with ImplicitAssigns. Callee could be unanalysed because of an unanalysed indirect call, or a "recursion
	// break".
	ReturnStatement *calleeReturn;

public:
	/***************************************************************************/ /**
	* \fn         CallStatement::CallStatement
	* \brief         Constructor for a call
	******************************************************************************/
	CallStatement();
	
	/***************************************************************************/ /**
	* \fn      CallStatement::~CallStatement
	* \brief      Destructor
	******************************************************************************/
	virtual ~CallStatement();

	virtual void setNumber(int num) override;

	// Make a deep copy, and make the copy a derived object if needed.
	/***************************************************************************/ /**
	* \fn        CallStatement::clone
	* \brief     Deep copy clone
	* \returns   Pointer to a new Statement, a clone of this CallStatement
	******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this stmt
	// visit this stmt
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	/***************************************************************************/ /**
	* \fn      CallStatement::setArguments
	* \brief      Set the arguments of this call.
	* \param      args - the list of locations to set the arguments to (for testing)
	******************************************************************************/
	void setArguments(StatementList& args);

	// Set implicit arguments: so far, for testing only:
	// void        setImpArguments(std::vector<Exp*>& arguments);
	//        void        setReturns(std::vector<Exp*>& returns);// Set call's return locs
	
	/***************************************************************************/ /**
	* \fn      CallStatement::setSigArguments
	* \brief   Set the arguments of this call based on signature info
	* \note    Should only be called for calls to library functions
	******************************************************************************/
	void setSigArguments();                             // Set arguments based on signature

	StatementList& getArguments() { return arguments; } // Return call's arguments
	void updateArguments();                             // Update the arguments based on a callee change

	// Exp        *getDefineExp(int i);
	/// Temporarily needed for ad-hoc type analysis
	int findDefine(SharedExp e);        // Still needed temporarily for ad hoc type analysis
	void removeDefine(SharedExp e);
	void addDefine(ImplicitAssign *as); // For testing

	// void        ignoreReturn(SharedExp e);
	// void        ignoreReturn(int n);
	// void        addReturn(SharedExp e, Type* ty = nullptr);
	
	/// Set the defines to the set of locations modified by the callee,
	/// or if no callee, to all variables live at this call
	void updateDefines();         // Update the defines based on a callee change
	
	// Calculate results(this) = defines(this) intersect live(this)
	// Note: could use a LocationList for this, but then there is nowhere to store the types (for DFA based TA)
	// So the RHS is just ignored
	StatementList *calcResults(); // Calculate defines(this) isect live(this)

	ReturnStatement *getCalleeReturn() { return calleeReturn; }
	void setCalleeReturn(ReturnStatement *ret) { calleeReturn = ret; }
	bool isChildless() const;
	SharedExp getProven(SharedExp e);

	std::shared_ptr<Signature> getSignature() { return signature; }
	void setSignature(std::shared_ptr<Signature> sig) { signature = sig; } ///< Only used by range analysis
	
	/// Localise the various components of expression e with reaching definitions to this call
	/// Note: can change e so usually need to clone the argument
	/// Was called substituteParams
	///
	/// Substitute the various components of expression e with the appropriate reaching definitions.
	/// Used in e.g. fixCallBypass (via the CallBypasser). Locations defined in this call are replaced with their proven
	/// values, which are in terms of the initial values at the start of the call (reaching definitions at the call)
	SharedExp localiseExp(SharedExp e);
	
	/// Localise only components of e, i.e. xxx if e is m[xxx]
	void localiseComp(SharedExp e); // Localise only xxx of m[xxx]

	// Do the call bypass logic e.g. r28{20} -> r28{17} + 4 (where 20 is this CallStatement)
	// Set ch if changed (bypassed)
	SharedExp bypassRef(const std::shared_ptr<RefExp>& r, bool& ch);

	void clearUseCollector() { useCol.clear(); }
	void addArgument(SharedExp e, UserProc *proc);
	
	/// Find the reaching definition for expression e.
	/// Find the definition for the given expression, using the embedded Collector object
	/// Was called findArgument(), and used implicit arguments and signature parameters
	/// \note must only operator on unsubscripted locations, otherwise it is invalid
	SharedExp findDefFor(SharedExp e);
	SharedExp getArgumentExp(int i);
	void setArgumentExp(int i, SharedExp e);
	void setNumArguments(int i);
	int getNumArguments();
	void removeArgument(int i);
	SharedType getArgumentType(int i);
	void setArgumentType(int i, SharedType ty);
	void truncateArguments();
	void clearLiveEntry();
	void eliminateDuplicateArgs();

	virtual void print(QTextStream& os, bool html = false) const override;

	// general search
	virtual bool search(const Exp& search, SharedExp& result) const override;

	// Replace all instances of "search" with "replace".
	/***************************************************************************/ /**
	* \fn              CallStatement::searchAndReplace
	* \brief           Replace all instances of search with replace.
	* \param search  - a location to search for
	* \param replace - the expression with which to replace it
	* \param cc -      true to replace in collectors
	* \returns         True if any change
	******************************************************************************/
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.
	
	/***************************************************************************/ /**
	* \fn    CallStatement::searchAll
	* \brief Find all instances of the search expression
	* \param search - a location to search for
	* \param result - a list which will have any matching exprs appended to it
	* \returns true if there were any matches
	******************************************************************************/
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// Set and return whether the call is effectively followed by a return.
	// E.g. on Sparc, whether there is a restore in the delay slot.
	/***************************************************************************/ /**
	* \fn    CallStatement::setReturnAfterCall
	* \brief Sets a bit that says that this call is effectively followed by a return. This happens e.g. on
	*        Sparc when there is a restore in the delay slot of the call
	* \param b: true if this is to be set; false to clear the bit
	******************************************************************************/
	void setReturnAfterCall(bool b);
	
	/***************************************************************************/ /**
	* \fn    CallStatement::isReturnAfterCall
	* \brief Tests a bit that says that this call is effectively followed by a return. This happens e.g. on
	*        Sparc when there is a restore in the delay slot of the call
	* \returns True if this call is effectively followed by a return
	******************************************************************************/
	bool isReturnAfterCall() const;

	// Set and return the list of Exps that occur *after* the call (the
	// list of exps in the RTL occur before the call). Useful for odd patterns.
	void setPostCallExpList(std::list<SharedExp> *le);

	std::list<SharedExp> *getPostCallExpList();

	// Set and return the destination proc.
	/***************************************************************************/ /**
	* \brief        Set the destination of this jump to be a given expression.
	* \param        pd - the new target
	******************************************************************************/
	void setDestProc(Function *dest);
	Function *getDestProc();

	// Generate constraints
	virtual void genConstraints(LocationSet& cons) override;

	// Data flow based type analysis
	void dfaTypeAnalysis(bool& ch) override;

	// code generation
	virtual void generateCode(HLLCode *hll, BasicBlock *Parent, int indLevel) override;

	// dataflow analysis
	virtual bool usesExp(const Exp& e) const override;

	// dataflow related functions
	virtual bool isDefinition() const override;
	virtual void getDefinitions(LocationSet& defs) const override;

	/// Does a ReturnStatement define anything? Not really, the locations are already defined earlier in the procedure.
	/// However, nothing comes after the return statement, so it doesn't hurt to pretend it does, and this is a place to
	/// store the return type(s) for example.
	/// FIXME: seems it would be cleaner to say that Return Statements don't define anything.
	virtual bool definesLoc(SharedExp loc) const override; // True if this Statement defines loc

	// get how to replace this statement in a use
	// virtual Exp*        getRight() { return nullptr; }

	// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	//        void        setIgnoreReturnLoc(bool b);

	void decompile();

	// Insert actual arguments to match formal parameters
	// void        insertArguments(InstructionSet& rs);

	virtual SharedType getTypeFor(SharedExp e) const override;     // Get the type defined by this Statement for this location
	virtual void setTypeFor(SharedExp e, SharedType ty) override;  // Set the type for this location, defined in this statement

	DefCollector *getDefCollector() { return &defCol; } // Return pointer to the def collector object
	UseCollector *getUseCollector() { return &useCol; }    // Return pointer to the use collector object
	void useBeforeDefine(SharedExp x) { useCol.insert(x); } // Add x to the UseCollector for this call
	void removeLiveness(SharedExp e) { useCol.remove(e); } // Remove e from the UseCollector
	void removeAllLive() { useCol.clear(); }               // Remove all livenesses
	//        Exp*        fromCalleeContext(Exp* e);            // Convert e from callee to caller (this) context
	StatementList& getDefines() { return defines; } // Get list of locations defined by this call
	
	/// Process this call for ellipsis parameters. If found, in a printf/scanf call, truncate the number of
	/// parameters if needed, and return true if any signature parameters added
	/// This function has two jobs. One is to truncate the list of arguments based on the format string.
	/// The second is to add parameter types to the signature.
	/// If -Td is used, type analysis will be rerun with these changes.
	bool ellipsisProcessing(Prog *prog);
	
	/// Attempt to convert this call, if indirect, to a direct call.
	/// NOTE: at present, we igore the possibility that some other statement
	/// will modify the global. This is a serious limitation!!
	bool convertToDirect(); // Internal function: attempt to convert an indirect to a

	// direct call
	void useColFromSsaForm(Instruction *s) { useCol.fromSSAform(m_proc, s); }

	bool isCallToMemOffset() const;

private:
	// Private helper functions for the above
	// Helper function for makeArgAssign(?)
	void addSigParam(SharedType ty, bool isScanf);
	
	/// Make an assign suitable for use as an argument from a callee context expression
	Assign *makeArgAssign(SharedType ty, SharedExp e);
	bool objcSpecificProcessing(const QString& formatStr);

protected:
	void updateDefineWithType(int n);

	void appendArgument(Assignment *as) { arguments.append(as); }
	friend class XMLProgParser;
}; // class CallStatement

/*===========================================================
* ReturnStatement: represents an ordinary high level return.
*==========================================================*/
class ReturnStatement : public Instruction
{
public:
	typedef StatementList::iterator iterator;
	typedef StatementList::const_iterator const_iterator;
	
protected:
	// Native address of the (only) return instruction. Needed for branching to this only return statement
	ADDRESS retAddr;

	/**
	 * The progression of return information is as follows:
	 * First, reaching definitions are collected in the DefCollector col. These are not sorted or filtered.
	 * Second, some of those definitions make it to the modifieds list, which is sorted and filtered. These are
	 * the locations that are modified by the enclosing procedure. As locations are proved to be preserved (with NO
	 * modification, not even sp = sp+4), they are removed from this list. Defines in calls to the enclosing
	 * procedure are based on this list.
	 * Third, the modifications are initially copied to the returns list (also sorted and filtered, but the returns
	 * have RHS where the modifieds don't). Locations not live at any caller are removed from the returns, but not
	 * from the modifieds.
	 */
	/// A DefCollector object to collect the reaching definitions
	DefCollector col;

	/// A list of assignments that represents the locations modified by the enclosing procedure. These assignments
	/// have no RHS?
	/// These transmit type information to callers
	/// Note that these include preserved locations early on (?)
	StatementList modifieds;

	/// A list of assignments of locations to expressions.
	/// Initially definitions reaching the exit less preserveds; later has locations unused by any callers removed.
	/// A list is used to facilitate ordering. (A set would be ideal, but the ordering depends at runtime on the
	/// signature)
	StatementList returns;

public:
	ReturnStatement();
	virtual ~ReturnStatement();
	
	iterator begin() { return returns.begin(); }
	iterator end()   { return returns.end(); }
	
	const_iterator begin() const { return returns.begin(); }
	const_iterator end()   const { return returns.end(); }
	
	iterator erase(iterator it) { return returns.erase(it); }
	
	StatementList& getModifieds() { return modifieds; }
	StatementList& getReturns() { return returns; }
	
	size_t getNumReturns() const { return returns.size(); }
	
	// Update the modifieds, in case the signature and hence ordering and filtering has changed, or the locations in the
	// collector have changed. Does NOT remove preserveds (deferred until updating returns).
	void updateModifieds(); // Update modifieds from the collector
	
	// Update the returns, in case the signature and hence ordering
	// and filtering has changed, or the locations in the modifieds list
	void updateReturns();   // Update returns from the modifieds

	virtual void print(QTextStream& os, bool html = false) const override;

	// general search
	virtual bool search(const Exp&, SharedExp&) const override;

	// Replace all instances of "search" with "replace".
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// Searches for all instances of a given subexpression within this statement and adds them to a given list
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// returns true if this statement uses the given expression
	virtual bool usesExp(const Exp& e) const override;

	virtual void getDefinitions(LocationSet& defs) const override;

	void removeModified(SharedExp loc); // Remove from modifieds AND from returns
	
	// Remove the return (if any) related to loc. Loc may or may not be subscripted
	void removeReturn(SharedExp loc);   // Remove from returns only
	void addReturn(Assignment *a);

	/// Scan the returns for e. If found, return the type associated with that return
	SharedType getTypeFor(SharedExp e) const override;
	void setTypeFor(SharedExp e, SharedType ty) override;

	// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	virtual bool isDefinition() const override { return true; }

	// Get a subscripted version of e from the collector
	SharedExp subscriptWithDef(SharedExp e);

	// Make a deep copy, and make the copy a derived object if needed.
	/***************************************************************************/ /**
	* \brief        Deep copy clone
	* \returns             Pointer to a new Statement, a clone of this ReturnStatement
	******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this Statement
	// visit this stmt
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	virtual bool definesLoc(SharedExp loc) const override; // True if this Statement defines loc

	// code generation
	virtual void generateCode(HLLCode *hll, BasicBlock *Parent, int indLevel) override;

	// Exp        *getReturnExp(int n) { return returns[n]; }
	// void        setReturnExp(int n, SharedExp e) { returns[n] = e; }
	// void        setSigArguments();                     // Set returns based on signature
	DefCollector *getCollector() { return &col; } // Return pointer to the collector object

	// Get and set the native address for the first and only return statement
	ADDRESS getRetAddr() { return retAddr; }
	void setRetAddr(ADDRESS r) { retAddr = r; }

	// Find definition for e (in the collector)
	SharedExp findDefFor(SharedExp e) { return col.findDefFor(e); }

	void dfaTypeAnalysis(bool& ch) override;

	// Remove the stack pointer and return a statement list
	StatementList *getCleanReturns();

	// Temporary hack (not neccesary anymore)
	// void        specialProcessing();

	friend class XMLProgParser;
}; // class ReturnStatement
