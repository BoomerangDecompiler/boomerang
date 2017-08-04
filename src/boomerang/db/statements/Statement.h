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

#include "boomerang/db/exp/ExpHelp.h"  // For lessExpStar, lessAssignment etc
#include "boomerang/db/DataFlow.h" // For embedded objects DefCollector and UseCollector#

#include "boomerang/db/Managed.h"

#include "boomerang/util/Address.h"

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
class Statement;
class Signature;
class StmtVisitor;
class StmtExpVisitor;
class StmtModifier;
class StmtPartModifier;
class ICodeGenerator;
class Assign;
class RTL;
class InstructionSet;
class ReturnStatement;

typedef std::set<UserProc *>           CycleSet;
typedef std::shared_ptr<Exp>           SharedExp;
typedef std::unique_ptr<Statement>   UniqInstruction;
typedef std::shared_ptr<Type>          SharedType;

/***************************************************************************/ /**
 * Types of Statements, or high-level register transfer lists.
 * changing the order of these will result in save files not working - trent
 ******************************************************************************/
enum StmtType : uint8_t   // ' : uint8_t' so that it can be forward declared in rtl.h
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
enum BranchType
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
 * Class hierarchy:   Statement@            (@ = abstract)
 *                  __/   |   \________________________
 *                 /      |            \               \
 *     GotoStatement  TypingStatement@  ReturnStatement JunctionStatement
 * BranchStatement_/     /          \
 * CaseStatement__/  Assignment@   ImpRefStatement
 * CallStatement_/  /   /    \ \________
 *       PhiAssign_/ Assign  BoolAssign \_ImplicitAssign
 */
class Statement
{
    typedef std::map<SharedExp, int, lessExpStar> ExpIntMap;

public:
    Statement()
        : m_parent(nullptr)
        , m_proc(nullptr)
        , m_number(0) {}

    virtual ~Statement() {}

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

    StmtType getKind() const { return m_kind; }
    void setKind(StmtType k) { m_kind = k; }

    virtual Statement *clone() const = 0;  ///< Make copy of self

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
    bool isFlagAssign() const;                                   ///< true if this statment is a flags assignment

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
    bool propagateTo(bool& convert, ExpIntMap *destCounts = nullptr, LocationSet *usedByDomPhi = nullptr,
                     bool force = false);

    /// Experimental: may want to propagate flags first,
    /// without tests about complexity or the propagation limiting heuristic
    bool propagateFlagsTo();

    // code generation
    virtual void generateCode(ICodeGenerator *hll, BasicBlock *Parent, int indLevel) = 0;
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
    Statement *getNextStatementInBB() const;
    Statement *getPreviousStatementInBB() const;

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
    void subscriptVar(SharedExp e, Statement *def /*, Cfg* cfg */);

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

    StmtType m_kind; // Statement kind (e.g. STMT_BRANCH)
    unsigned int m_lexBegin, m_lexEnd;
};


/// Print the Statement (etc) pointed to by p
QTextStream& operator<<(QTextStream& os, const Statement *p);
QTextStream& operator<<(QTextStream& os, const InstructionSet *p);
QTextStream& operator<<(QTextStream& os, const LocationSet *p);



/***************************************************************************/ /**
 * CaseStatement is derived from GotoStatement. In addition to the destination
 * of the jump, it has a switch variable Exp.
 ******************************************************************************/
struct SWITCH_INFO
{
    SharedExp pSwitchVar;  ///< Ptr to Exp repres switch var, e.g. v[7]
    char      chForm;      ///< Switch form: 'A', 'O', 'R', 'H', or 'F' etc
    int       iLower;      ///< Lower bound of the switch variable
    int       iUpper;      ///< Upper bound for the switch variable
    Address   uTable;      ///< Native address of the table, or ptr to array of values for form F
    int       iNumTable;   ///< Number of entries in the table (form H only)
    int       iOffset = 0; ///< Distance from jump to table (form R only)
};
