#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/db/exp/ExpHelp.h"
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
class LocationSet;

typedef std::shared_ptr<Exp>         SharedExp;
typedef std::shared_ptr<Type>        SharedType;


/// Types of Statements, or high-level register transfer lists.
enum class StmtType : uint8_t
{
    INVALID = 0,
    Assign = 1,
    PhiAssign,  ///< x := phi(a, b, c)
    ImpAssign,
    BoolAssign, ///< For "setCC" instructions
    Call,
    Ret,        ///< Return
    Branch,
    Goto,
    Case,       ///< switch statement
    ImpRef,
    Junction
};

/**
 * These values indicate what kind of conditional jump or
 * conditonal assign is being performed.
 */
enum class BranchType : uint8_t
{
    INVALID = 0,
    JE = 1, ///< Jump if equals
    JNE,    ///< Jump if not equals
    JSL,    ///< Jump if signed less
    JSLE,   ///< Jump if signed less or equal
    JSGE,   ///< Jump if signed greater or equal
    JSG,    ///< Jump if signed greater
    JUL,    ///< Jump if unsigned less
    JULE,   ///< Jump if unsigned less or equal
    JUGE,   ///< Jump if unsigned greater or equal
    JUG,    ///< Jump if unsigned greater
    JMI,    ///< Jump if result is minus
    JPOS,   ///< Jump if result is positive
    JOF,    ///< Jump if overflow
    JNOF,   ///< Jump if no overflow
    JPAR    ///< Jump if parity even (Intel only)
};


/**
 * Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 * Class hierarchy:
 *                    Statement@            (@ = abstract)
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
    Statement();

    virtual ~Statement() = default;

    /// Make copy of self, and make the copy a derived object if needed.
    virtual Statement *clone() const = 0;

    /// \returns the BB that this statement is part of.
    BasicBlock *getBB() { return m_parent; }
    const BasicBlock *getBB() const { return m_parent; }

    /// Changes the BB that this statment is part of.
    void setBB(BasicBlock *bb) { m_parent = bb; }

    /// \returns the procedure this statement is part of.
    UserProc *getProc() const { return m_proc; }

    /// Changes the procedure this statement is part of.
    void setProc(UserProc *p);

    int getNumber() const { return m_number; }

    /// Overridden for calls (and maybe later returns)
    virtual void setNumber(int num) { m_number = num; }

    StmtType getKind() const { return m_kind; }
    void setKind(StmtType k) { m_kind = k; }

    /// Accept a visitor (of various kinds) to this Statement.
    /// \return true to continue visiting
    virtual bool accept(StmtVisitor *visitor)       = 0;
    virtual bool accept(StmtExpVisitor *visitor)    = 0;
    virtual bool accept(StmtModifier *modifier)     = 0;
    virtual bool accept(StmtPartModifier *modifier) = 0;

    void setLexBegin(unsigned int n) { m_lexBegin = n; }
    void setLexEnd(unsigned int n) { m_lexEnd = n; }
    unsigned int getLexBegin() const { return m_lexBegin; }
    unsigned int getLexEnd() const { return m_lexEnd; }

    /// returns true if this statement defines anything
    virtual bool isDefinition() const = 0;

    /// true if is a null statement
    bool isNullStatement() const;

    /// Return true if a TypingStatement
    virtual bool isTyping() const { return false; }

    /// true if this statement is a standard assign
    bool isAssign() const { return m_kind == StmtType::Assign; }

    /// true if this statement is a any kind of assignment
    bool isAssignment() const
    {
        return m_kind == StmtType::Assign
            || m_kind == StmtType::PhiAssign
            || m_kind == StmtType::ImpAssign
            || m_kind == StmtType::BoolAssign;
    }

    /// \returns true if this statement is a phi assignment
    bool isPhi() const { return m_kind == StmtType::PhiAssign; }

    /// \returns true if this statement is an implicit assignment
    bool isImplicit() const { return m_kind == StmtType::ImpAssign; }

    /// \returns true if this statment is a flags assignment
    bool isFlagAssign() const;

    /// \returns true if this statement is an implicit reference
    bool isImpRef() const { return m_kind == StmtType::ImpRef; }

    virtual bool isGoto()   { return m_kind == StmtType::Goto; }
    virtual bool isBranch() { return m_kind == StmtType::Branch; }

    /// \returns true if this statement is a junction
    bool isJunction() const { return m_kind == StmtType::Junction; }

    /// \returns true if this statement is a call
    bool isCall() const { return m_kind == StmtType::Call; }

    /// \returns true if this statement is a BoolAssign
    bool isBool() const { return m_kind == StmtType::BoolAssign; }

    /// \returns true if this statement is a ReturnStatement
    bool isReturn() const { return m_kind == StmtType::Ret; }

    /// true if this statement is a decoded ICT.
    /// \note for now, it only represents decoded indirect jump instructions
    bool isHL_ICT() const { return m_kind == StmtType::Case; }

    bool isCase() { return m_kind == StmtType::Case; }

    /// \returns true if this is a fpush/fpop
    bool isFpush() const;
    bool isFpop() const;

    /// Classes with no definitions (e.g. GotoStatement and children) don't override this
    /// returns a set of locations defined by this statement in a LocationSet argument.
    virtual void getDefinitions(LocationSet& /*def*/) const {}

    virtual bool definesLoc(SharedExp /*loc*/) const { return false; }  // True if this Statement defines loc

    /// returns true if this statement uses the given expression
    virtual bool usesExp(const Exp& exp) const = 0;

    /**
     * Display a text reprentation of this statement to the given stream
     * \note  Usually called from RTL::print, in which case the first 9
     *        chars of the print have already been output to os
     * \param os - stream to write to
     * \param html - produce html encoded representation
     */
    virtual void print(QTextStream& os, bool html = false) const = 0;

    void printAsUse(QTextStream& os) const { os << m_number; }
    void printAsUseBy(QTextStream& os) const { os << m_number; }
    void printNum(QTextStream& os) const { os << m_number; }
    char *prints() const; // For logging, was also for debugging

    // This version prints much better in gdb
    void dump() const;    // For debugging

    /// general search
    virtual bool search(const Exp& pattern, SharedExp& result) const = 0;

    /**
     * Find all instances of \p pattern and adds all found expressions
     * to \p result in reverse nesting order.
     *
     * \param   pattern an expression to search for
     * \param   result  a list which will have any matching exps
     *                  appended to it in reverse nesting order.
     * \returns true if there were any matches
     */
    virtual bool searchAll(const Exp& pattern, std::list<SharedExp>& result) const = 0;

    /**
     * Replace all instances of search with replace.
     * \param pattern a location to search for
     * \param replace the expression with which to replace it
     * \param cc      Set to true to change collectors as well.
     * \returns True if any change
     */
    virtual bool searchAndReplace(const Exp& pattern, SharedExp replace, bool cc = false) = 0; // TODO: consider constness

    /**
     * \returns true if can propagate to \p exp (must be a RefExp to return true)
     * \note does not consider whether e is able to be renamed
     * (from a memory Primitive point of view),
     * only if the definition can be propagated TO this stmt
     */
    static bool canPropagateToExp(Exp& exp);

    /**
     * Propagate to this statement.
     * \param destCounts is a map that indicates how may times a statement's definition is used
     * \param convert set true if an indirect call is changed to direct (otherwise, no change)
     * \param force set to true to propagate even memofs (for switch analysis)
     * \param usedByDomPhi is a set of subscripted locations used in phi statements
     * \returns true if a change
     */
    bool propagateTo(bool& convert, ExpIntMap *destCounts = nullptr, LocationSet *usedByDomPhi = nullptr,
                     bool force = false);

    /// Experimental: may want to propagate flags first,
    /// without tests about complexity or the propagation limiting heuristic
    bool propagateFlagsTo();

    /// Generate code for this statement
    virtual void generateCode(ICodeGenerator *gen, const BasicBlock *parentBB) = 0;

    /// simpify internal expressions
    virtual void simplify() = 0;

    /// simplify internal address expressions (a[m[x]] -> x) etc
    /// Only Assignments override at present
    virtual void simplifyAddr() {}

    /// map registers and temporaries to local variables
    void mapRegistersToLocals();

    /// The last part of the fromSSA logic: replace subscripted locations with suitable local variables
    void replaceSubscriptsWithLocals();

    /// insert casts where needed, since fromSSA will erase type information
    void insertCasts();

    // Only Assign overrides at present
    virtual void fixSuccessor() {}

    // Data flow based type analysis
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
    BasicBlock *m_parent = nullptr; ///< contains a pointer to the enclosing BB
    UserProc *m_proc = nullptr;     ///< procedure containing this statement
    int m_number = -1;              ///< Statement number for printing

#if USE_DOMINANCE_NUMS
    int m_dominanceNum = -1;        ///< Like a statement number, but has dominance properties

public:
    int getDomNumber() const { return m_dominanceNum; }
    void setDomNumber(int dn) { m_dominanceNum = dn; }

protected:
#endif

    StmtType m_kind = StmtType::INVALID; ///< Statement kind (e.g. STMT_BRANCH)
    unsigned int m_lexBegin, m_lexEnd;
};


/**
 * Output operator for Statement *.
 * Just makes it easier to use e.g. LOG_STREAM() << myStmtStar
 * \param os output stream to send to
 * \param stmt  ptr to Statement to print to the stream
 * \returns copy of os (for concatenation)
 */
QTextStream& operator<<(QTextStream& os, const Statement *stmt);
QTextStream& operator<<(QTextStream& os, const InstructionSet *p);
QTextStream& operator<<(QTextStream& os, const LocationSet *p);


/**
 * CaseStatement is derived from GotoStatement. In addition to the destination
 * of the jump, it has a switch variable Exp.
 */
struct SwitchInfo
{
    SharedExp pSwitchVar;  ///< Ptr to Exp repres switch var, e.g. v[7]
    char      chForm;      ///< Switch form: 'A', 'O', 'R', 'H', or 'F' etc
    int       iLower;      ///< Lower bound of the switch variable
    int       iUpper;      ///< Upper bound for the switch variable
    Address   uTable;      ///< Native address of the table, or ptr to array of values for form F
    int       iNumTable;   ///< Number of entries in the table (form H only)
    int       iOffset = 0; ///< Distance from jump to table (form R only)
};
