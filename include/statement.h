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
  * OVERVIEW:   The Statement and related classes (was dataflow.h)
  ******************************************************************************/

#ifndef _STATEMENT_H_
#define _STATEMENT_H_

/* Class hierarchy:   Statement@            (@ = abstract)
                    __/   |   \________________________
                   /      |            \               \
       GotoStatement  TypingStatement@  ReturnStatement JunctionStatement
 BranchStatement_/     /          \
 CaseStatement__/  Assignment@   ImpRefStatement
 CallStatement_/  /   /    \ \________
       PhiAssign_/ Assign  BoolAssign \_ImplicitAssign
*/
//#include "exp.h"        // No! This is (almost) the bottom of the #include hierarchy
#include "config.h"

#include "memo.h"
#include "exphelp.h" // For lessExpStar, lessAssignment etc
#include "types.h"
#include "managed.h"
#include "dataflow.h"  // For embedded objects DefCollector and UseCollector
//#include "boomerang.h" // For USE_DOMINANCE_NUMS etc

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

typedef std::set<UserProc *> CycleSet;
typedef std::shared_ptr<Exp> SharedExp;
typedef std::unique_ptr<Instruction> UniqInstruction;
typedef std::shared_ptr<Type> SharedType;
/***************************************************************************/ /**
  * Kinds of Statements, or high-level register transfer lists.
  * changing the order of these will result in save files not working - trent
  ******************************************************************************/
enum STMT_KIND : uint8_t { // ' : uint8_t' so that it can be forward declared in rtl.h
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
    STMT_JUNCTION };

/***************************************************************************/ /**
  * BRANCH_TYPE: These values indicate what kind of conditional jump or
  * conditonal assign is being performed.
  * Changing the order of these will result in save files not working - trent
  ******************************************************************************/
enum BRANCH_TYPE {
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
class Instruction {
protected:
    typedef std::map<Exp *, int, lessExpStar> mExpInt;
    BasicBlock *Parent; // contains a pointer to the enclosing BB
    UserProc *proc;     // procedure containing this statement
    int Number;         // Statement number for printing
#if USE_DOMINANCE_NUMS
    int DominanceNum; // Like a statement number, but has dominance properties
public:
    int getDomNumber() { return DominanceNum; }
    void setDomNumber(int dn) { DominanceNum = dn; }

protected:
#endif
    STMT_KIND Kind; // Statement kind (e.g. STMT_BRANCH)
    unsigned int LexBegin, LexEnd;

public:
    Instruction() : Parent(nullptr), proc(nullptr), Number(0) {} //, parent(nullptr)
    virtual ~Instruction() {}

    // get/set the enclosing BB, etc
    BasicBlock *getBB() { return Parent; }
    const BasicBlock *getBB() const { return Parent; }
    void setBB(BasicBlock *bb) { Parent = bb; }

    //        bool        operator==(Statement& o);
    // Get and set *enclosing* proc (not destination proc)
    void setProc(UserProc *p);
    UserProc *getProc() { return proc; }

    int getNumber() const { return Number; }
    virtual void setNumber(int num) { Number = num; } // Overridden for calls (and maybe later returns)

    STMT_KIND getKind() const { return Kind; }
    void setKind(STMT_KIND k) { Kind = k; }

    virtual Instruction * clone() const = 0; // Make copy of self

    // Accept a visitor (of various kinds) to this Statement. Return true to continue visiting
    virtual bool accept(StmtVisitor *visitor) = 0;
    virtual bool accept(StmtExpVisitor *visitor) = 0;
    virtual bool accept(StmtModifier *visitor) = 0;
    virtual bool accept(StmtPartModifier *visitor) = 0;

    void setLexBegin(unsigned int n) { LexBegin = n; }
    void setLexEnd(unsigned int n) { LexEnd = n; }
    unsigned int getLexBegin() { return LexBegin; }
    unsigned int getLexEnd() { return LexEnd; }

    //! returns true if this statement defines anything
    virtual bool isDefinition() = 0;
    bool isNullStatement(); //!< true if is a null statement
    virtual bool isTyping() { return false; } // Return true if a TypingStatement
    //! true if this statement is a standard assign
    bool isAssign() const { return Kind == STMT_ASSIGN; }
    //! true if this statement is a any kind of assignment
    bool isAssignment() {
        return Kind == STMT_ASSIGN || Kind == STMT_PHIASSIGN || Kind == STMT_IMPASSIGN || Kind == STMT_BOOLASSIGN;
    }

    bool isPhi() const { return Kind == STMT_PHIASSIGN; }   //!< true    if this statement is a phi assignment
    bool isImplicit() const { return Kind == STMT_IMPASSIGN; }  //!< true if this statement is an implicit assignment
    bool isFlagAssgn(); //!< true if this statment is a flags assignment
    bool isImpRef() const { return Kind == STMT_IMPREF; } //!< true of this statement is an implicit reference

    virtual bool isGoto() { return Kind == STMT_GOTO; }
    virtual bool isBranch() { return Kind == STMT_BRANCH; }

    // true if this statement is a junction
    bool isJunction() const { return Kind == STMT_JUNCTION; }

    //! true if this statement is a call
    bool isCall() { return Kind == STMT_CALL; }

    //! true if this statement is a BoolAssign
    bool isBool() { return Kind == STMT_BOOLASSIGN; }

    //! true if this statement is a ReturnStatement
    bool isReturn() { return Kind == STMT_RET; }

    //! true if this statement is a decoded ICT.
    //! \note for now, it only represents decoded indirect jump instructions
    bool isHL_ICT() { return Kind == STMT_CASE; }

    bool isCase() { return Kind == STMT_CASE; }

    //! true if this is a fpush/fpop
    bool isFpush();
    bool isFpop();

    //! Classes with no definitions (e.g. GotoStatement and children) don't override this
    //! returns a set of locations defined by this statement in a LocationSet argument.
    virtual void getDefinitions(LocationSet & /*def*/) {}

    // set the left for forExp to newExp

    virtual bool definesLoc(Exp * /*loc*/) { return false; } // True if this Statement defines loc

    // returns true if this statement uses the given expression
    virtual bool usesExp(const Exp &) = 0;

    // statements should be printable (for debugging)
    virtual void print(QTextStream &os, bool html = false) const = 0;
    void printAsUse(QTextStream &os) { os <<  Number; }
    void printAsUseBy(QTextStream &os) { os <<  Number; }
    void printNum(QTextStream &os) { os << Number; }
    char *prints(); // For logging, was also for debugging
    void dump();    // For debugging

    // general search
    virtual bool search(const Exp &search, Exp *&result) = 0;
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result) = 0;

    // general search and replace. Set cc true to change collectors as well. Return true if any change
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false) = 0; // TODO: consider constness

    // True if can propagate to expression e in this Statement.
    static bool canPropagateToExp(Exp &e);
    bool propagateTo(bool &convert, mExpInt *destCounts = nullptr, LocationSet *usedByDomPhi = nullptr,
                     bool force = false);
    bool propagateFlagsTo();

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *Parent, int indLevel) = 0;
    virtual void simplify() = 0; //!< simpify internal expressions

    //! simplify internal address expressions (a[m[x]] -> x) etc
    //! Only Assignments override at present
    virtual void simplifyAddr() {}

    //! map registers and temporaries to local variables
    void mapRegistersToLocals();

    //! The last part of the fromSSA logic: replace subscripted locations with suitable local variables
    void replaceSubscriptsWithLocals();

    //! insert casts where needed, since fromSSA will erase type information
    void insertCasts();

    // fixSuccessor
    // Only Assign overrides at present
    virtual void fixSuccessor() {}

    //! Generate constraints (for constraint based type analysis)
    virtual void genConstraints(LocationSet & /*cons*/) {}

    // Data flow based type analysis
    virtual void dfaTypeAnalysis(bool & /*ch*/) {} // Use the type information in this Statement
    SharedType meetWithFor(SharedType ty, Exp *e, bool &ch); // Meet the type associated with e with ty

public:

    // helper functions
    bool isFirstStatementInBB();
    bool isLastStatementInBB();
    Instruction *getNextStatementInBB();
    Instruction *getPreviousStatementInBB();

    //    //    //    //    //    //    //    //    //    //
    //                                                    //
    //    Statement visitation functions                  //
    //                                                    //
    //    //    //    //    //    //    //    //    //    //

    void addUsedLocs(LocationSet &used, bool cc = false, bool memOnly = false);
    bool addUsedLocals(LocationSet &used);
    void bypass();
    bool replaceRef(Exp *e, Assign *def, bool &convert);
    void findConstants(std::list<Const *> &lc);
    int setConscripts(int n);
    void clearConscripts();
    void stripSizes();
    void subscriptVar(Exp *e, Instruction *def /*, Cfg* cfg */);

    // Cast the constant num to type ty. If a change was made, return true
    bool castConst(int num, SharedType ty);

    // Map expressions to locals
    void dfaMapLocals();

    // End Statement visitation functions

    //! Get the type for the definition, if any, for expression e in this statement
    //! Overridden only by Assignment and CallStatement, and ReturnStatement.
    virtual SharedType getTypeFor(Exp *) { return nullptr; }
    //! Set the type for the definition of e in this Statement
    virtual void setTypeFor(Exp *, SharedType ) { assert(0); }

    // virtual    Type*    getType() {return nullptr;}            // Assignment, ReturnStatement and
    // virtual    void    setType(Type* t) {assert(0);}        // CallStatement override

    bool doPropagateTo(Exp *e, Assign *def, bool &convert);
    bool calcMayAlias(Exp *e1, Exp *e2, int size);
    bool mayAlias(Exp *e1, Exp *e2, int size);

    friend class XMLProgParser;
}; // class Statement

// Print the Statement (etc) poited to by p
QTextStream &operator<<(QTextStream &os, const Instruction *p);
QTextStream &operator<<(QTextStream &os, const InstructionSet *p);
QTextStream &operator<<(QTextStream &os, const LocationSet *p);

/***************************************************************************/ /**
 * TypingStatement is an abstract subclass of Statement. It has a type, representing the type of a reference or an
 * assignment
 ****************************************************************************/
class TypingStatement : public Instruction {
protected:
    SharedType type; // The type for this assignment or reference
public:
    TypingStatement(SharedType ty); // Constructor

    // Get and set the type.
    SharedType getType() { return type; }
    const SharedType &getType() const { return type; }
    void setType(SharedType ty) { type = ty; }

    virtual bool isTyping() { return true; }
};

/***************************************************************************/ /**
 * Assignment is an abstract subclass of TypingStatement, holding a location
 ****************************************************************************/
class Assignment : public TypingStatement {
protected:
    Exp *lhs; // The left hand side
public:
    // Constructor, subexpression
    Assignment(Exp *lhs);
    // Constructor, type, and subexpression
    Assignment(SharedType ty, Exp *lhs);
    // Destructor
    virtual ~Assignment();

    // Clone
    virtual Instruction * clone() const = 0;

    // We also want operator< for assignments. For example, we want ReturnStatement to contain a set of (pointers
    // to) Assignments, so we can automatically make sure that existing assignments are not duplicated
    // Assume that we won't want sets of assignments differing by anything other than LHSs
    bool operator<(const Assignment &o) { return lhs < o.lhs; }

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor) = 0;
    virtual bool accept(StmtExpVisitor *visitor) = 0;
    virtual bool accept(StmtModifier *visitor) = 0;
    virtual bool accept(StmtPartModifier *visitor) = 0;

    virtual void print(QTextStream &os, bool html = false) const;
    virtual void printCompact(QTextStream &os, bool html = false) const = 0; // Without statement number

    virtual SharedType getTypeFor(Exp *e);          // Get the type for this assignment. It should define e
    virtual void setTypeFor(Exp *e, SharedType ty); // Set the type for this assignment. It should define e

    virtual bool usesExp(const Exp &e); // PhiAssign and ImplicitAssign don't override

    virtual bool isDefinition() { return true; }
    virtual void getDefinitions(LocationSet &defs);
    virtual bool definesLoc(Exp *loc); // True if this Statement defines loc

    // get how to access this lvalue
    virtual Exp *getLeft() { return lhs; } // Note: now only defined for Assignments, not all Statements
    virtual const Exp *getLeft() const { return lhs; }

    virtual Exp *getRight() = 0;
    // set the lhs to something new
    void setLeft(Exp *e) { lhs = e; }

    // memory depth
    int getMemDepth();

    // general search
    virtual bool search(const Exp &search, Exp *&result) = 0;
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result) = 0;

    // general search and replace
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false) = 0;
    void generateCode(HLLCode *, BasicBlock *, int /*indLevel*/) {}

    // simpify internal expressions
    virtual void simplify() = 0;

    // simplify address expressions
    virtual void simplifyAddr();

    // generate Constraints
    virtual void genConstraints(LocationSet &cons);

    // Data flow based type analysis
    void dfaTypeAnalysis(bool &ch);

    friend class XMLProgParser;
}; // class Assignment

/***************************************************************************/ /**
 * Assign an ordinary assignment with left and right sides
 ***************************************************************************/
class Assign : public Assignment {
    Exp *rhs;
    Exp *guard;

public:
    // Constructor, subexpressions
    Assign(Exp *lhs, Exp *r, Exp *guard = nullptr);
    // Constructor, type and subexpressions
    Assign(SharedType ty, Exp *lhs, Exp *r, Exp *guard = nullptr);
    // Default constructor, for XML parser
    Assign() : Assignment(nullptr), rhs(nullptr), guard(nullptr) {}
    // Copy constructor
    Assign(Assign &o);
    // Destructor
    ~Assign() {}

    // Clone
    virtual Instruction * clone() const override;

    // get how to replace this statement in a use
    virtual Exp *getRight() override { return rhs; }
    Exp *&getRightRef() { return rhs; }

    // set the rhs to something new
    void setRight(Exp *e) { rhs = e; }

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    virtual void printCompact(QTextStream &os, bool html = false) const override; // Without statement number

    // Guard
    void setGuard(Exp *g) { guard = g; }
    Exp *getGuard() { return guard; }
    bool isGuarded() { return guard != nullptr; }

    virtual bool usesExp(const Exp &e) override;
    virtual bool isDefinition()  override { return true; }

    // general search
    virtual bool search(const Exp &search, Exp *&result) override;
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result) override;

    // general search and replace
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false) override;

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
    virtual void genConstraints(LocationSet &cons) override;

    // Data flow based type analysis
    void dfaTypeAnalysis(bool &ch);

    // FIXME: I suspect that this was only used by adhoc TA, and can be deleted
    bool match(const char *pattern, std::map<QString, Exp *> &bindings);

    friend class XMLProgParser;
}; // class Assign

// The below could almost be a RefExp. But we could not at one stage #include exp.h as part of statement,h; that's since
// changed so it is now possible, and arguably desirable.  However, it's convenient to have these members public
struct PhiInfo {
    // A default constructor is required because CFG changes (?) can cause access to elements of the vector that
    // are beyond the current end, creating gaps which have to be initialised to zeroes so that they can be skipped
    PhiInfo() {} //: def(0), e(0) not initializing to help valgrind find locations of unset vals
    Exp *e;      // The expression for the thing being defined (never subscripted)
    void def(Instruction *def) { m_def = def; /*assert(def);*/ }
    Instruction *def() { return m_def; }
    const Instruction *def() const { return m_def; }

protected:
    Instruction *m_def; // The defining statement
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
class PhiAssign : public Assignment {
public:
    typedef std::map<BasicBlock *, PhiInfo> Definitions;
    typedef Definitions::iterator iterator;
    typedef Definitions::const_iterator const_iterator;

private:
    Definitions DefVec; // A vector of information about definitions
public:
    PhiAssign(Exp *lhs) : Assignment(lhs) { Kind = STMT_PHIASSIGN; }
    PhiAssign(SharedType ty, Exp *lhs) : Assignment(ty, lhs) { Kind = STMT_PHIASSIGN; }
    // Copy constructor (not currently used or implemented)
    PhiAssign(Assign &o);
    virtual ~PhiAssign() {}

    // Clone
    virtual Instruction * clone() const;

    // get how to replace this statement in a use
    virtual Exp *getRight() { return nullptr; }

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor);
    virtual bool accept(StmtExpVisitor *visitor);
    virtual bool accept(StmtModifier *visitor);
    virtual bool accept(StmtPartModifier *visitor);

    virtual void printCompact(QTextStream &os, bool html = false) const;

    // general search
    virtual bool search(const Exp &search, Exp *&result);
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result);

    // general search and replace
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false);

    // simplify all the uses/defs in this Statement
    virtual void simplify();

    // Generate constraints
    virtual void genConstraints(LocationSet &cons);

    // Data flow based type analysis
    void dfaTypeAnalysis(bool &ch);

    //
    //    Phi specific functions
    //

    // Get or put the statement at index idx
    Instruction *getStmtAt(BasicBlock *idx) {
        if (DefVec.find(idx) == DefVec.end()) {
            return nullptr;
        }
        return DefVec[idx].def();
    }
    PhiInfo &getAt(BasicBlock *idx);
    void putAt(BasicBlock *idx, Instruction *d, Exp *e);
    void simplifyRefs();
    virtual size_t getNumDefs() { return DefVec.size(); }
    Definitions &getDefs() { return DefVec; }
    // A hack. Check MVE
    bool hasGlobalFuncParam();

    PhiInfo &front() { return DefVec.begin()->second; }
    PhiInfo &back() { return DefVec.rbegin()->second; }
    iterator begin() { return DefVec.begin(); }
    iterator end() { return DefVec.end(); }
    const_iterator cbegin() const { return DefVec.begin(); }
    const_iterator cend() const { return DefVec.end(); }
    iterator erase(iterator it) { return DefVec.erase(it); }

    // Convert this phi assignment to an ordinary assignment
    void convertToAssign(Exp *rhs);

    // Generate a list of references for the parameters
    void enumerateParams(std::list<Exp *> &le);

protected:
    friend class XMLProgParser;
}; // class PhiAssign

// An implicit assignment has only a left hand side. It is a placeholder for storing the types of parameters and
// globals.  That way, you can always find the type of a subscripted variable by looking in its defining Assignment
class ImplicitAssign : public Assignment {
public:
    ImplicitAssign(Exp *lhs);
    ImplicitAssign(SharedType ty, Exp *lhs);
    ImplicitAssign(ImplicitAssign &o);
    virtual ~ImplicitAssign();

    virtual Instruction * clone() const;
    void dfaTypeAnalysis(bool &ch);

    // general search
    virtual bool search(const Exp &search, Exp *&result);
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result);

    // general search and replace
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false);

    virtual void printCompact(QTextStream &os, bool html = false) const;

    // Statement and Assignment functions
    virtual Exp *getRight() { return nullptr; }
    virtual void simplify() {}

    // Visitation
    virtual bool accept(StmtVisitor *visitor);
    virtual bool accept(StmtExpVisitor *visitor);
    virtual bool accept(StmtModifier *visitor);
    virtual bool accept(StmtPartModifier *visitor);

}; // class ImplicitAssign

/***************************************************************************/ /**
  * BoolAssign represents "setCC" type instructions, where some destination is set (to 1 or 0) depending on the
  * condition codes. It has a condition Exp, similar to the BranchStatement class.
  * *==========================================================================*/
class BoolAssign : public Assignment {
    BRANCH_TYPE jtCond; // the condition for setting true
    Exp *pCond;         // Exp representation of the high level
    // condition: e.g. r[8] == 5
    bool bFloat;        // True if condition uses floating point CC
    int Size;           // The size of the dest
public:
    BoolAssign(int size);
    virtual ~BoolAssign();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Instruction * clone() const;

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor);
    virtual bool accept(StmtExpVisitor *visitor);
    virtual bool accept(StmtModifier *visitor);
    virtual bool accept(StmtPartModifier *visitor);

    // Set and return the BRANCH_TYPE of this scond as well as whether the
    // floating point condition codes are used.
    void setCondType(BRANCH_TYPE cond, bool usesFloat = false);
    BRANCH_TYPE getCond() { return jtCond; }
    bool isFloat() { return bFloat; }
    void setFloat(bool b) { bFloat = b; }

    // Set and return the Exp representing the HL condition
    Exp *getCondExpr();
    void setCondExpr(Exp *pss);
    // As above, no delete (for subscripting)
    void setCondExprND(Exp *e) { pCond = e; }
    int getSize() { return Size; } // Return the size of the assignment
    void makeSigned();

    virtual void printCompact(QTextStream &os, bool html = false) const;
    virtual void generateCode(HLLCode *hll, BasicBlock *, int indLevel);
    virtual void simplify();

    // Statement functions
    virtual bool isDefinition() { return true; }
    virtual void getDefinitions(LocationSet &def);
    virtual Exp *getRight() { return getCondExpr(); }
    virtual bool usesExp(const Exp &e);
    virtual bool search(const Exp &search, Exp *&result);
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result);
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false);
    // a hack for the SETS macro
    void setLeftFromList(std::list<Instruction *> *stmts);

    virtual void dfaTypeAnalysis(bool &ch);

    friend class XMLProgParser;
}; // class BoolAssign

// An implicit reference has only an expression. It holds the type information that results from taking the address
// of a location. Note that dataflow can't decide which local variable (in the decompiled output) is being taken,
// if there is more than one local variable sharing the same memory address (separated then by type).
class ImpRefStatement : public TypingStatement {
    Exp *addressExp; // The expression representing the address of the location referenced
public:
    // Constructor, subexpression
    ImpRefStatement(SharedType ty, Exp *a) : TypingStatement(ty), addressExp(a) { Kind = STMT_IMPREF; }
    Exp *getAddressExp() { return addressExp; }
    SharedType getType() { return type; }
    void meetWith(SharedType ty, bool &ch); // Meet the internal type with ty. Set ch if a change

    // Virtuals
    virtual Instruction * clone() const override;
    virtual bool accept(StmtVisitor *) override;
    virtual bool accept(StmtExpVisitor *) override;
    virtual bool accept(StmtModifier *) override;
    virtual bool accept(StmtPartModifier *) override;
    virtual bool isDefinition()  override { return false; }
    virtual bool usesExp(const Exp &)  override { return false; }
    virtual bool search(const Exp &, Exp *&) override;
    virtual bool searchAll(const Exp &, std::list<Exp *, std::allocator<Exp *>> &) override;
    virtual bool searchAndReplace(const Exp &, Exp *, bool cc = false) override;
    virtual void generateCode(HLLCode *, BasicBlock *, int)  override {}
    virtual void simplify() override;
    virtual void print(QTextStream &os, bool html = false) const override;

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
class GotoStatement : public Instruction {
protected:
    Exp *pDest;        // Destination of a jump or call. This is the absolute destination for both
    // static and dynamic CTIs.
    bool m_isComputed; // True if this is a CTI with a computed destination address.
    // NOTE: This should be removed, once CaseStatement and HLNwayCall are implemented
    // properly.
    Const *constDest() { return ((Const *)pDest); }
    const Const *constDest() const { return ((const Const *)pDest); }

public:
    GotoStatement();
    GotoStatement(ADDRESS jumpDest);
    virtual ~GotoStatement();

    virtual Instruction * clone() const override; //!< Make a deep copy, and make the copy a derived object if needed.

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    // Set and return the destination of the jump. The destination is either an Exp, or an ADDRESS that
    // is converted to a Exp.
    void setDest(Exp *pd);
    void setDest(ADDRESS addr);
    virtual Exp *getDest();
    virtual const Exp *getDest() const;

    ADDRESS getFixedDest() const;
    void adjustFixedDest(int delta);

    // Set and return whether the destination of this CTI is computed.
    // NOTE: These should really be removed, once CaseStatement and HLNwayCall are implemented properly.
    void setIsComputed(bool b = true);
    bool isComputed();

    virtual void print(QTextStream &os, bool html = false) const override;

    // general search
    virtual bool search(const Exp &, Exp *&) override;

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false) override;

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result) override;

    // code generation
    virtual void generateCode(HLLCode *, BasicBlock *, int) override;

    // simplify all the uses/defs in this Statement
    virtual void simplify() override;

    // Statement virtual functions
    virtual bool isDefinition()  override { return false; }
    virtual bool usesExp(const Exp &) override;

    friend class XMLProgParser;
}; // class GotoStatement

class JunctionStatement : public Instruction {
public:
    JunctionStatement() { Kind = STMT_JUNCTION; }

    Instruction * clone() const override { return new JunctionStatement(); }

    // Accept a visitor (of various kinds) to this Statement. Return true to continue visiting
    bool accept(StmtVisitor *visitor) override;
    bool accept(StmtExpVisitor *visitor) override;
    bool accept(StmtModifier *visitor) override;
    bool accept(StmtPartModifier *visitor) override;

    // returns true if this statement defines anything
    bool isDefinition()  override { return false; }

    bool usesExp(const Exp &)  override { return false; }

    void print(QTextStream &os, bool html = false) const override;

    // general search
    bool search(const Exp & /*search*/, Exp *& /*result*/)  override { return false; }
    bool searchAll(const Exp & /*search*/, std::list<Exp *> & /*result*/)  override { return false; }

    //! general search and replace. Set cc true to change collectors as well. Return true if any change
    bool searchAndReplace(const Exp & /*search*/, Exp * /*replace*/, bool /*cc*/ = false)  override { return false; }

    void generateCode(HLLCode * /*hll*/, BasicBlock * /*pbb*/, int /*indLevel*/)  override {}

    // simpify internal expressions
    void simplify() override {}

    bool isLoopJunction() const;
};

/***************************************************************************/ /**==
  * BranchStatement has a condition Exp in addition to the destination of the jump.
  *==============================================================================*/
class BranchStatement : public GotoStatement {
    BRANCH_TYPE jtCond; // The condition for jumping
    Exp * pCond;         // The Exp representation of the high level condition: e.g., r[8] == 5
    bool bFloat;        // True if uses floating point CC
    // jtCond seems to be mainly needed for the Pentium weirdness.
    // Perhaps bFloat, jtCond, and size could one day be merged into a type
    int size;         // Size of the operands, in bits

public:
    BranchStatement();
    virtual ~BranchStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Instruction * clone() const override;

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    // Set and return the BRANCH_TYPE of this jcond as well as whether the
    // floating point condition codes are used.
    void setCondType(BRANCH_TYPE cond, bool usesFloat = false);
    BRANCH_TYPE getCond() { return jtCond; }
    bool isFloat() { return bFloat; }
    void setFloat(bool b) { bFloat = b; }

    // Set and return the Exp representing the HL condition
    Exp *getCondExpr();
    void setCondExpr(Exp *pe);

    BasicBlock *getFallBB();
    BasicBlock *getTakenBB();
    void setFallBB(BasicBlock *bb);
    void setTakenBB(BasicBlock *bb);

    // Probably only used in front386.cc: convert this from an unsigned to a
    // signed conditional branch
    void makeSigned();

    virtual void print(QTextStream &os, bool html = false) const override;

    // general search
    virtual bool search(const Exp &search, Exp *&result)  override;

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false) override;

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result) override;

    // code generation
    virtual void generateCode(HLLCode *, BasicBlock *, int) override;

    // dataflow analysis
    virtual bool usesExp(const Exp &e) override;

    // simplify all the uses/defs in this Statememt
    virtual void simplify() override;

    // Generate constraints
    virtual void genConstraints(LocationSet &cons) override;

    // Data flow based type analysis
    void dfaTypeAnalysis(bool &ch);

    friend class XMLProgParser;
}; // class BranchStatement

/***************************************************************************/ /**
  * CaseStatement is derived from GotoStatement. In addition to the destination
  * of the jump, it has a switch variable Exp.
  ******************************************************************************/
struct SWITCH_INFO {
    Exp *pSwitchVar; // Ptr to Exp repres switch var, e.g. v[7]
    char chForm;     // Switch form: 'A', 'O', 'R', 'H', or 'F' etc
    int iLower;      // Lower bound of the switch variable
    int iUpper;      // Upper bound for the switch variable
    ADDRESS uTable;  // Native address of the table, or ptr to array of values for form F
    int iNumTable;   // Number of entries in the table (form H only)
    int iOffset = 0; // Distance from jump to table (form R only)
    // int        delta;            // Host address - Native address
};

class CaseStatement : public GotoStatement {
    SWITCH_INFO *pSwitchInfo; // Ptr to struct with info about the switch
public:
    CaseStatement();
    virtual ~CaseStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Instruction * clone() const;

    // Accept a visitor to this Statememt
    virtual bool accept(StmtVisitor *visitor);
    virtual bool accept(StmtExpVisitor *visitor);
    virtual bool accept(StmtModifier *visitor);
    virtual bool accept(StmtPartModifier *visitor);

    // Set and return the Exp representing the switch variable
    SWITCH_INFO *getSwitchInfo();
    void setSwitchInfo(SWITCH_INFO *psi);

    virtual void print(QTextStream &os, bool html = false) const;

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false);

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result);

    // code generation
    virtual void generateCode(HLLCode *, BasicBlock *, int);

    // dataflow analysis
    virtual bool usesExp(const Exp &e);

public:
    // simplify all the uses/defs in this Statement
    virtual void simplify();

    friend class XMLProgParser;
}; // class CaseStatement

/***************************************************************************/ /**
  * CallStatement: represents a high level call. Information about parameters and the like are stored here.
  ******************************************************************************/
class CallStatement : public GotoStatement {
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
    Signature *signature;

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
    CallStatement();
    virtual ~CallStatement();

    virtual void setNumber(int num) override;
    // Make a deep copy, and make the copy a derived object if needed.
    virtual Instruction * clone() const override;

    // Accept a visitor to this stmt
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    void setArguments(StatementList &args);
    // Set implicit arguments: so far, for testing only:
    // void        setImpArguments(std::vector<Exp*>& arguments);
    //        void        setReturns(std::vector<Exp*>& returns);// Set call's return locs
    void setSigArguments();                             // Set arguments based on signature
    StatementList &getArguments() { return arguments; } // Return call's arguments
    void updateArguments();                             // Update the arguments based on a callee change
    // Exp        *getDefineExp(int i);
    int findDefine(Exp *e); // Still needed temporarily for ad hoc type analysis
    void removeDefine(Exp *e);
    void addDefine(ImplicitAssign *as); // For testing
    // void        ignoreReturn(Exp *e);
    // void        ignoreReturn(int n);
    // void        addReturn(Exp *e, Type* ty = nullptr);
    void updateDefines();         // Update the defines based on a callee change
    StatementList *calcResults(); // Calculate defines(this) isect live(this)
    ReturnStatement *getCalleeReturn() { return calleeReturn; }
    void setCalleeReturn(ReturnStatement *ret) { calleeReturn = ret; }
    bool isChildless() const;
    Exp *getProven(Exp *e);
    Signature *getSignature() { return signature; }
    void setSignature(Signature *sig) { signature = sig;} ///< Only used by range analysis
    // Localise the various components of expression e with reaching definitions to this call
    // Note: can change e so usually need to clone the argument
    // Was called substituteParams
    Exp *localiseExp(Exp *e);
    void localiseComp(Exp *e); // Localise only xxx of m[xxx]
    // Do the call bypass logic e.g. r28{20} -> r28{17} + 4 (where 20 is this CallStatement)
    // Set ch if changed (bypassed)
    Exp *bypassRef(RefExp *r, bool &ch);
    void clearUseCollector() { useCol.clear(); }
    void addArgument(Exp *e, UserProc *proc);
    Exp *findDefFor(Exp *e); // Find the reaching definition for expression e
    Exp *getArgumentExp(int i);
    void setArgumentExp(int i, Exp *e);
    void setNumArguments(int i);
    int getNumArguments();
    void removeArgument(int i);
    SharedType getArgumentType(int i);
    void setArgumentType(int i, SharedType ty);
    void truncateArguments();
    void clearLiveEntry();
    void eliminateDuplicateArgs();

    virtual void print(QTextStream &os, bool html = false) const override;

    // general search
    virtual bool search(const Exp &search, Exp *&result) override;

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false) override;

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result) override;

    // Set and return whether the call is effectively followed by a return.
    // E.g. on Sparc, whether there is a restore in the delay slot.
    void setReturnAfterCall(bool b);
    bool isReturnAfterCall();

    // Set and return the list of Exps that occur *after* the call (the
    // list of exps in the RTL occur before the call). Useful for odd patterns.
    void setPostCallExpList(std::list<Exp *> *le);
    std::list<Exp *> *getPostCallExpList();

    // Set and return the destination proc.
    void setDestProc(Function *dest);
    Function *getDestProc();

    // Generate constraints
    virtual void genConstraints(LocationSet &cons) override;

    // Data flow based type analysis
    void dfaTypeAnalysis(bool &ch);

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *Parent, int indLevel) override;

    // dataflow analysis
    virtual bool usesExp(const Exp &e) override;

    // dataflow related functions
    virtual bool isDefinition() override;
    virtual void getDefinitions(LocationSet &defs) override;

    virtual bool definesLoc(Exp *loc) override; // True if this Statement defines loc

    // get how to replace this statement in a use
    // virtual Exp*        getRight() { return nullptr; }

    // simplify all the uses/defs in this Statement
    virtual void simplify() override;

    //        void        setIgnoreReturnLoc(bool b);

    void decompile();

    // Insert actual arguments to match formal parameters
    // void        insertArguments(InstructionSet& rs);

    virtual SharedType getTypeFor(Exp *e) override;             // Get the type defined by this Statement for this location
    virtual void setTypeFor(Exp *e, SharedType ty) override;    // Set the type for this location, defined in this statement
    DefCollector *getDefCollector() { return &defCol; } // Return pointer to the def collector object
    UseCollector *getUseCollector() { return &useCol; } // Return pointer to the use collector object
    void useBeforeDefine(Exp *x) { useCol.insert(x); }  // Add x to the UseCollector for this call
    void removeLiveness(Exp *e) { useCol.remove(e); }   // Remove e from the UseCollector
    void removeAllLive() { useCol.clear(); }            // Remove all livenesses
    //        Exp*        fromCalleeContext(Exp* e);            // Convert e from callee to caller (this) context
    StatementList &getDefines() { return defines; } // Get list of locations defined by this call
    // Process this call for ellipsis parameters. If found, in a printf/scanf call, truncate the number of
    // parameters if needed, and return true if any signature parameters added
    bool ellipsisProcessing(Prog *prog);
    bool convertToDirect(); // Internal function: attempt to convert an indirect to a
    // direct call
    void useColFromSsaForm(Instruction *s) { useCol.fromSSAform(proc, s); }

    bool isCallToMemOffset() const;
private:
    // Private helper functions for the above
    void addSigParam(SharedType ty, bool isScanf);
    Assign *makeArgAssign(SharedType ty, Exp *e);
    bool objcSpecificProcessing(const QString &formatStr);

protected:
    void updateDefineWithType(int n);
    void appendArgument(Assignment *as) { arguments.append(as); }
    friend class XMLProgParser;
}; // class CallStatement

/*===========================================================
 * ReturnStatement: represents an ordinary high level return.
 *==========================================================*/
class ReturnStatement : public Instruction {
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

    typedef StatementList::iterator iterator;
    iterator begin() { return returns.begin(); }
    iterator end() { return returns.end(); }
    iterator erase(iterator it) { return returns.erase(it); }
    StatementList &getModifieds() { return modifieds; }
    StatementList &getReturns() { return returns; }
    size_t getNumReturns() { return returns.size(); }
    void updateModifieds(); // Update modifieds from the collector
    void updateReturns();   // Update returns from the modifieds

    virtual void print(QTextStream &os, bool html = false) const override;

    // general search
    virtual bool search(const Exp &, Exp *&) override;

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(const Exp &search, Exp *replace, bool cc = false) override;

    // Searches for all instances of a given subexpression within this statement and adds them to a given list
    virtual bool searchAll(const Exp &search, std::list<Exp *> &result) override;

    // returns true if this statement uses the given expression
    virtual bool usesExp(const Exp &e) override;

    virtual void getDefinitions(LocationSet &defs) override;

    void removeModified(Exp *loc); // Remove from modifieds AND from returns
    void removeReturn(Exp *loc);   // Remove from returns only
    void addReturn(Assignment *a);

    SharedType getTypeFor(Exp *e) override;
    void setTypeFor(Exp *e, SharedType ty) override;

    // simplify all the uses/defs in this Statement
    virtual void simplify() override;

    virtual bool isDefinition()  override { return true; }

    // Get a subscripted version of e from the collector
    Exp *subscriptWithDef(Exp *e);

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Instruction * clone() const override;

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    virtual bool definesLoc(Exp *loc) override; // True if this Statement defines loc

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *Parent, int indLevel) override;

    // Exp        *getReturnExp(int n) { return returns[n]; }
    // void        setReturnExp(int n, Exp *e) { returns[n] = e; }
    // void        setSigArguments();                     // Set returns based on signature
    DefCollector *getCollector() { return &col; } // Return pointer to the collector object

    // Get and set the native address for the first and only return statement
    ADDRESS getRetAddr() { return retAddr; }
    void setRetAddr(ADDRESS r) { retAddr = r; }

    // Find definition for e (in the collector)
    Exp *findDefFor(Exp *e) { return col.findDefFor(e); }

    virtual void dfaTypeAnalysis(bool &ch);

    // Remove the stack pointer and return a statement list
    StatementList *getCleanReturns();

    // Temporary hack (not neccesary anymore)
    // void        specialProcessing();

    friend class XMLProgParser;
}; // class ReturnStatement

#endif // __STATEMENT_H__
