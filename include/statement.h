/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       statement.h
 * OVERVIEW:   The Statement and related classes (was dataflow.h)
 *============================================================================*/

/*
 * $Revision$
 * 25 Nov 02 - Trent: appropriated for use by new dataflow.
 * 3 July 02 - Trent: created.
 * 03 Feb 03 - Mike: cached dataflow (uses and usedBy)
 * 03 Apr 03 - Mike: Added StatementSet
 * 25 Jul 03 - Mike: Changed dataflow.h to statement.h
 */

#ifndef _STATEMENT_H_
#define _STATEMENT_H_

/* Class hierarchy:           Statement (abstract)
                              /  |  \ 
                             /   |   \
                 GotoStatement Assign BoolStatement
         _______/   |   \____ \____________       
        /           |        \             \     
BranchStatement CaseStatement CallStatement ReturnStatement
*/

#include <vector>
#include <set>
#include <list>
#include <map>
#include <ostream>
#include <iostream>     // For std::cerr
//#include "exp.h"      // No! This is the bottom of the #include hierarchy
#include "exphelp.h"    // For lessExpStar
#include "types.h"
#include "managed.h"

class BasicBlock;
typedef BasicBlock *PBB;
class Prog;
class Proc;
class UserProc;
class Exp;
class RefExp;
class Cfg;
class Type;
class Statement;
class StmtVisitor;
class HLLCode;
class Assign;
class RTL;
typedef std::map<Exp*, int, lessExpStar> igraph;

/*==============================================================================
 * Kinds of Statements, or high-level register transfer lists.
 * changing the order of these will result in save files not working - trent
 *============================================================================*/
enum STMT_KIND {
    STMT_ASSIGN = 0,
    STMT_CALL,
    STMT_RET,
    STMT_BRANCH,
    STMT_GOTO,
    STMT_BOOL,                  // For "setCC" instructions that set destination
                                // to 1 or 0 depending on the condition codes.
    STMT_CASE,                  // Used to represent switch statements.
};

//  //  //  //  //  //  //  //  //  //  //  //  //  //
//
//  A b s t r a c t   C l a s s   S t a t e m e n t //
//
//  //  //  //  //  //  //  //  //  //  //  //  //  //

/* Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 */
class Statement {
protected:
    PBB     pbb;        // contains a pointer to the enclosing BB
    UserProc *proc;     // procedure containing this statement
    int     number;     // Statement number for printing
    STMT_KIND kind;     // Statement kind (e.g. STMT_BRANCH)
    Statement *parent;  // The statement that contains this one
public:

    Statement() : pbb(NULL), proc(NULL), number(0), parent(NULL) { }
    virtual ~Statement() {
    }

    bool        operator==(Statement& o);
    void        setProc(UserProc *p);
    UserProc*   getProc() {return proc;}

    virtual Statement*  clone() = 0;            // Make copy of self

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor* visitor) = 0;

    STMT_KIND getKind() { return kind;}
    void setKind(STMT_KIND k) {kind = k;}

    // returns true if this statement defines anything
    virtual bool isDefinition() = 0;

    // true if is a null statement
    bool    isNullStatement();

    // true if this statement is any kind of assign
    bool isAssign() {return kind == STMT_ASSIGN;}

    virtual bool isGoto() { return kind == STMT_GOTO; }
    virtual bool isBranch() { return kind == STMT_BRANCH; }

    // true if this statement is an ordinary (non flags) assignment
    bool isOrdinaryAssign();

    // true if this statment is a flags assignment
    bool isFlagAssgn();

    // true if this statement is a phi assignment
    bool isPhi();

    // true if this statement is a call
    bool isCall() { return kind == STMT_CALL; }

    // true if this statement is a BoolStatement
    bool isBool() { return kind == STMT_BOOL; }

    bool isReturn() { return kind == STMT_RET; }

    // true if this is a fpush/fpop
    bool isFpush();
    bool isFpop();

    // returns a set of locations defined by this statement
    // Classes with no definitions (e.g. GotoStatement and children) don't
    // override this
    virtual void getDefinitions(LocationSet &def) {}

    // returns an expression that would be used to reference the value
    // defined by this statement (if this statement is propogatable)
    virtual Exp* getLeft() = 0;

    // returns a type for the left
    virtual Type* getLeftType() = 0;

    // returns an expression that would be used to replace this statement
    // in a use
    virtual Exp* getRight() = 0;

    // returns true if this statement uses the given expression
    virtual bool usesExp(Exp *e) = 0;

    // Adds (inserts) all locations (registers or memory) used by this
    // statement
    virtual void addUsedLocs(LocationSet& used) = 0;

    virtual void fixCallRefs() = 0;

    // Subscript all occurrences of e with definition def (except for top level
    // of LHS of assignment)
    virtual void subscriptVar(Exp* e, Statement* def) = 0;

    // returns the statement which is used by this statement and has a
    // left like the given expression
    // MVE: is this useful?
    //virtual Statement *findDef(Exp *e);

    // 
    // get my uses' definitions (ud chain)
    // 
    //void calcUses(StatementSet &uses);
    //int getNumUses() { return uses.size(); }
    //StatementSet &getUses() { return uses; }
    //void clearUses() {uses.clear(); usedBy.clear();}
 
    // 
    // usedBy: du chain (my def's uses)
    //
    //void calcUsedBy(StatementSet &usedBy);
    //int getNumUsedBy() { return usedBy.size(); }

    // update my data flow (I'm about to be deleted)
    //void updateDfForErase();

    // get/set the enclosing BB
    PBB getBB() { return pbb; }
    void setBB(PBB bb) { pbb = bb; }

    // replaces a use of the given statement with an expression
            void replaceRef(Statement *use);
    // special version of the above for the "special hack"
    // (see Proc::propagateStatements, where numUses == 2)
            void specialReplaceRef(Statement* def);

    // statements should be printable (for debugging)
    virtual void print(std::ostream &os, bool withUses = false) = 0;
    virtual void printWithUses(std::ostream& os) {print(os, true);}
            void printAsUse(std::ostream &os)   {os << std::dec << number;}
            void printAsUseBy(std::ostream &os) {os << std::dec << number;}
            void printNum(std::ostream &os)     {os << std::dec << number;}
            char* prints();      // For use in a debugger

    // inline / decode any constants in the statement
    virtual void processConstants(Prog *prog) = 0;

    // general search
    virtual bool search(Exp *search, Exp *&result) = 0;
    virtual bool searchAll(Exp* search, std::list<Exp*>& result) = 0;

    // general search and replace
    virtual bool searchAndReplace(Exp *search, Exp *replace) = 0;

    // update the type information for an expression in this statement
    virtual Type *updateType(Exp *e, Type *curType) = 0;

    // get the statement number
    virtual int getNumber() {return number;}

    // update the statement number
    void    setNumber(int num) {number = num;}

    // From SSA form
    virtual void fromSSAform(igraph& igm) = 0;

    // Propagate to this statement
    void propagateTo(int memDepth, StatementSet& exclude, int toDepth = -1);

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) = 0;

    // simpify internal expressions
    virtual void simplify() = 0;

    // simplify internal address expressions (a[m[x]] -> x)
    // Only Assign overrides at present
    virtual void simplifyAddr() {}

    // fixSuccessor
    // Only Assign overrides at present
    virtual void fixSuccessor() {}

    // Generate constraints
    virtual void genConstraints(LocationSet& cons) {}

protected:
    virtual void doReplaceRef(Exp* from, Exp* to) = 0;
    bool doPropagateTo(int memDepth, Statement* def);
    bool calcMayAlias(Exp *e1, Exp *e2, int size);
    bool mayAlias(Exp *e1, Exp *e2, int size);
    Exp *processConstant(Exp *e, Type *ty, Prog *prog);
    Type *getTypeFor(Exp *e, Prog *prog);
};          // class Statement

// Print the Statement (etc) poited to by p
std::ostream& operator<<(std::ostream& os, Statement* p);
std::ostream& operator<<(std::ostream& os, StatementSet* p);
std::ostream& operator<<(std::ostream& os, LocationSet* p);



/*==============================================================================
 * Assign is a subclass of Statement, holding two subexpressions and a size
 *============================================================================*/
class Assign : public Statement {
    Exp*    lhs;        // The left hand side
    Exp*    rhs;        // The right hand side
    int     size;       // The size
    Exp*    guard;      // Guard expression (if not NULL)
public:
    // Constructor
            Assign();
    // Constructor, subexpression
            Assign(Exp* lhs, Exp* rhs);
    // Constructor, size, and subexpressions.
            Assign(int sz, Exp* lhs, Exp* rhs);
    // Copy constructor
            Assign(Assign& o);

    // Clone
    virtual Statement* clone();

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor* visitor);

    // Compare
    bool    operator==(const Statement& o) const;
    bool    operator< (const Statement& o) const;

    virtual void print(std::ostream& os, bool withUses = false);
    void    appendDotFile(std::ofstream& of);

    // Get and set the size
    int     getSize();
    void    setSize(int sz);

    // Guard
    void setGuard(Exp* g) {guard = g;}
    Exp* getGuard() {return guard;}
    bool isGuarded() {return guard != NULL;}

    virtual bool usesExp(Exp *e);
    virtual void addUsedLocs(LocationSet& used);
    virtual void fixCallRefs();
    // Remove refs to statements defining restored locations

    virtual bool isDefinition() { return true; }
    virtual void getDefinitions(LocationSet &defs);
        
    // get how to access this value
    virtual Exp* getLeft() { return lhs; }
    virtual Type* getLeftType() { return NULL; }

    // get how to replace this statement in a use
    virtual Exp* getRight() { return rhs; }

    // set the rhs to something new
    void         setRight(Exp* e);

    // inline any constants in the statement
    virtual void processConstants(Prog *prog);

    // general search
    virtual bool search(Exp* search, Exp*& result);
    virtual bool searchAll(Exp* search, std::list<Exp*>& result);

    // general search and replace
    virtual bool searchAndReplace(Exp *search, Exp *replace);
 
    // update type for expression
    virtual Type *updateType(Exp *e, Type *curType);

    // subscript one variable
    void subscriptVar(Exp* e, Statement* def);

    // memory depth
    int getMemDepth();

    // from SSA form
    virtual void fromSSAform(igraph& ig);

    // Generate code
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

    // simpify internal expressions
    virtual void simplify();

    // simplify address expressions
    virtual void simplifyAddr();

    // fixSuccessor (succ(r2) -> r3)
    virtual void fixSuccessor();

    // generate Constraints
    virtual void genConstraints(LocationSet& cons);

protected:
    virtual void doReplaceRef(Exp* from, Exp* to);

};      // class Assign



/*=============================================================================
 * GotoStatement has just one member variable, a semantic string representing the
 * jump's destination (an integer constant for direct jumps; an expression
 * for register jumps). An instance of this class will never represent a
 * return or computed call as these are distinguised by the decoder and are
 * instantiated as CallStatements and ReturnStatements respecitvely. This class also
 * represents unconditional jumps with a fixed offset (e.g BN, Ba on SPARC).
 *===========================================================================*/
class GotoStatement: public Statement {
protected:
    Exp* pDest;                 // Destination of a jump or call. This is the
                                // absolute destination for both static and
                                // dynamic CTIs.
    bool m_isComputed;          // True if this is a CTI with a computed
                                // destination address. NOTE: This should be
                                // removed, once CaseStatement and HLNwayCall
                                // are implemented properly.
public:
    GotoStatement();
    GotoStatement(ADDRESS jumpDest);
    virtual ~GotoStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Statement* clone();

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor* visitor);

    // Set and return the destination of the jump. The destination is either
    // a Exp, or an ADDRESS that is converted to a Exp.
    void setDest(Exp* pd);
    void setDest(ADDRESS addr);
    virtual Exp* getDest();

    // Return the fixed destination of this CTI. For dynamic CTIs, returns -1.
    ADDRESS getFixedDest();

    // Adjust the fixed destination by a given amount. Invalid for dynamic CTIs.
    void adjustFixedDest(int delta);
    
    // Set and return whether the destination of this CTI is computed.
    // NOTE: These should really be removed, once CaseStatement and HLNwayCall
    // are implemented properly.
    void setIsComputed(bool b = true);
    bool isComputed();

    virtual void print(std::ostream& os = std::cout, bool withDF = false);

    // general search
    virtual bool search(Exp*, Exp*&);

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(Exp* search, Exp* replace);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.    
    virtual bool searchAll(Exp* search, std::list<Exp*> &result);

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

    // simplify all the uses/defs in this RTL
    virtual void simplify();

    // Statement virtual functions
    virtual bool isDefinition() { return false;}
    virtual Exp* getLeft() {return NULL;}
    virtual Type* getLeftType() {return NULL;};
    virtual Exp* getRight() {return NULL;}
    virtual bool usesExp(Exp*) {return false;}
    virtual void addUsedLocs(LocationSet&) {}
    virtual void fixCallRefs() { }
    virtual void subscriptVar(Exp*, Statement*) {}
    virtual void processConstants(Prog*) {}
    virtual Type* updateType(Exp* e, Type* curType) {return curType;}
    virtual void fromSSAform(igraph&) {}
    virtual void doReplaceRef(Exp*, Exp*) {}
};      // class GotoStatement

/*==============================================================================
 * BRANCH_TYPE: These values indicate what kind of conditional jump is being
 * performed.
 * changing the order of these will result in save files not working - trent
 *============================================================================*/
enum BRANCH_TYPE {
    BRANCH_JE = 0,          // Jump if equals
    BRANCH_JNE,             // Jump if not equals
    BRANCH_JSL,             // Jump if signed less
    BRANCH_JSLE,            // Jump if signed less or equal
    BRANCH_JSGE,            // Jump if signed greater or equal
    BRANCH_JSG,             // Jump if signed greater
    BRANCH_JUL,             // Jump if unsigned less
    BRANCH_JULE,            // Jump if unsigned less or equal
    BRANCH_JUGE,            // Jump if unsigned greater or equal
    BRANCH_JUG,             // Jump if unsigned greater
    BRANCH_JMI,             // Jump if result is minus
    BRANCH_JPOS,            // Jump if result is positive
    BRANCH_JOF,             // Jump if overflow
    BRANCH_JNOF,            // Jump if no overflow
    BRANCH_JPAR             // Jump if parity even (Intel only)
};


/*==============================================================================
 * BranchStatement has a condition Exp in addition to the destination of the jump.
 *============================================================================*/
class BranchStatement: public GotoStatement {
public:
    BranchStatement();
    virtual ~BranchStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Statement* clone();

    // Accept a visitor to this RTL
    virtual bool accept(StmtVisitor* visitor);

    // Set and return the BRANCH_TYPE of this jcond as well as whether the
    // floating point condition codes are used.
    void setCondType(BRANCH_TYPE cond, bool usesFloat = false);
    BRANCH_TYPE getCond(){ return jtCond; }
    bool isFloat(){ return bFloat; }
    void setFloat(bool b)      { bFloat = b; }

    // Set and return the Exp representing the HL condition
    Exp* getCondExpr();
    void setCondExpr(Exp* pe);
    // As above, no delete (for subscripting)
    void setCondExprND(Exp* e) { pCond = e; }
    
    // Probably only used in front386.cc: convert this from an unsigned to a
    // signed conditional branch
    void makeSigned();

    virtual void print(std::ostream& os = std::cout, bool withDF = false);
    virtual void print(std::ostream& os) { print(os, true); }

    // general search
    virtual bool search(Exp *search, Exp *&result);

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(Exp* search, Exp* replace);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    virtual bool searchAll(Exp* search, std::list<Exp*> &result);

#if 0
    // Used for type analysis. Stores type information that
    // can be gathered from the RTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);   
#endif

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

    // dataflow analysis
    virtual bool usesExp(Exp *e);
    virtual void addUsedLocs(LocationSet& used);
    virtual void fixCallRefs();
    virtual void subscriptVar(Exp* e, Statement* def);

    // dataflow related functions
    virtual bool canPropagateToAll() { return false; }
    virtual void propagateToAll() { assert(false); }

    virtual bool isDefinition() { return false; }

    // get how to access this value
    virtual Exp* getLeft() { return NULL; }
    virtual Type* getLeftType() { return NULL; }

    // get how to replace this statement in a use
    virtual Exp* getRight() { return pCond; }

    // inline any constants in the statement
    virtual void processConstants(Prog *prog);

    // simplify all the uses/defs in this RTL
    virtual void simplify();

    // update type for expression
    virtual Type *updateType(Exp *e, Type *curType);

    // From SSA form
    virtual void fromSSAform(igraph& ig);

protected:
    virtual void doReplaceRef(Exp* from, Exp* to);

private:
    BRANCH_TYPE jtCond;          // The condition for jumping
    Exp* pCond;              // The Exp representation of the high level
                                // condition: e.g., r[8] == 5
    bool bFloat;                // True if uses floating point CC

};      // class BranchStatement

/*==============================================================================
 * CaseStatement is derived from GotoStatement. In addition to the destination
 * of the jump, it has a switch variable Exp.
 *============================================================================*/
typedef struct {
    Exp* pSwitchVar;         // Ptr to Exp repres switch var, e.g. v[7]
    char    chForm;             // Switch form: 'A', 'O', 'R', or 'H'
    int     iLower;             // Lower bound of the switch variable
    int     iUpper;             // Upper bound for the switch variable
    ADDRESS uTable;             // Native address of the table
    int     iNumTable;          // Number of entries in the table (form H only)
    int     iOffset;            // Distance from jump to table (form R only)
    int     delta;              // Host address - Native address
} SWITCH_INFO;

class CaseStatement: public GotoStatement {
    SWITCH_INFO* pSwitchInfo;   // Ptr to struct with info about the switch
public:
    CaseStatement();
    virtual ~CaseStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Statement* clone();

    // Accept a visitor to this RTL
    virtual bool accept(StmtVisitor* visitor);

    // Set and return the Exp representing the switch variable
    SWITCH_INFO* getSwitchInfo(); 
    void setSwitchInfo(SWITCH_INFO* pss);
    
    virtual void print(std::ostream& os = std::cout, bool withDF = false);

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(Exp* search, Exp* replace);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    virtual bool searchAll(Exp* search, std::list<Exp*> &result);
    
#if 0
    // Used for type analysis. Stores type information that
    // can be gathered from the RTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);   
#endif     

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);
    
    // simplify all the uses/defs in this RTL
    virtual void simplify();

};          // class CaseStatement

/*==============================================================================
 * CallStatement: represents a high level call. Information about parameters and
 * the like are stored here.
 *============================================================================*/
class CallStatement: public GotoStatement {
    // FIXME: Below will likely go away soon...
    int returnTypeSize;         // Size in bytes of the struct, union or quad FP
                                // value returned by the called function.
    bool returnAfterCall;       // True if call is effectively followed by
                                // a return.
    
    // The list of arguments passed by this call
    std::vector<Exp*> arguments;
    // The list of arguments implicitly passed as a result of the calling 
    // convention of the called procedure or the actual arguments
    std::vector<Exp*> implicitArguments;

    // The set of locations that are defined by this call.
    std::vector<Exp*> returns;

    // Destination of call
    Proc* procDest;

public:
    CallStatement(int returnTypeSize = 0);
    virtual ~CallStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Statement* clone();

    // Accept a visitor to this RTL
    virtual bool accept(StmtVisitor* visitor);

    // Return true if the called function returns an aggregate: i.e., a
    // struct, union or quad floating point value.
    bool returnsStruct();

    void setArguments(std::vector<Exp*>& arguments); // Set call's arguments
    void setReturns(std::vector<Exp*>& returns); // Set call's return locs
    void setSigArguments();         // Set arguments based on signature
    std::vector<Exp*>& getArguments();            // Return call's arguments
    int getNumReturns();
    Exp *getReturnExp(int i);
    int findReturn(Exp *e);
    void removeReturn(Exp *e);
    void addReturn(Exp *e);
    Exp *getProven(Exp *e);
    Exp *substituteParams(Exp *e);
    Exp *findArgument(Exp *e);
    void addArgument(Exp *e);
    Exp* getArgumentExp(int i);
    Exp* getImplicitArgumentExp(int i);
    void setArgumentExp(int i, Exp *e);
    void setNumArguments(int i);
    int  getNumArguments();
    void removeArgument(int i);
    void removeImplicitArgument(int i);
    Type *getArgumentType(int i);
    void truncateArguments();
    void clearLiveEntry();

    //Exp* getReturnLoc();                // FIXME Get location used for return value

    virtual void print(std::ostream& os = std::cout, bool withDF = false);

    // general search
    virtual bool search(Exp *search, Exp *&result);

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(Exp* search, Exp* replace);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    virtual bool searchAll(Exp* search, std::list<Exp*> &result);

    // Set and return whether the call is effectively followed by a return.
    // E.g. on Sparc, whether there is a restore in the delay slot.
    void setReturnAfterCall(bool b);
    bool isReturnAfterCall();

    // Set and return the list of Exps that occur *after* the call (the
    // list of exps in the RTL occur before the call). Useful for odd patterns.
    void setPostCallExpList(std::list<Exp*>* le);
    std::list<Exp*>* getPostCallExpList();

    // Set and return the destination proc.
    void setDestProc(Proc* dest);
    Proc* getDestProc();

    // Generate constraints
    virtual void genConstraints(LocationSet& cons);

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

    // dataflow analysis
    virtual bool usesExp(Exp *e);
    virtual void addUsedLocs(LocationSet& used);
    virtual void fixCallRefs();
    virtual void subscriptVar(Exp* e, Statement* def);

    // dataflow related functions
    virtual void propagateToAll() { assert(false); }

    virtual bool isDefinition();
    virtual void getDefinitions(LocationSet &defs);

    // Note: CallStatement inherits getLeft() from GotoStatement (returns NULL)
    // Still called from (e.g.) UserProc::prover()
    //virtual Exp* getLeft() {assert(0);}
    // get how to replace this statement in a use
    virtual Exp* getRight() { return NULL; }

    // inline any constants in the statement
    virtual void processConstants(Prog *prog);

    // simplify all the uses/defs in this RTL
    virtual void simplify();

    // update type for expression
    virtual Type *updateType(Exp *e, Type *curType);

    void setIgnoreReturnLoc(bool b);

    void decompile();

    virtual void fromSSAform(igraph& ig);
        
    // Insert actual arguments to match formal parameters
    void    insertArguments(StatementSet& rs);

protected:
    virtual void doReplaceRef(Exp* from, Exp* to);

};      // class CallStatement


/*==============================================================================
 * ReturnStatement: represents a high level return.
 *============================================================================*/
class ReturnStatement: public GotoStatement {
protected:
    // number of bytes that this return pops
    int nBytesPopped;

    // value returned
    std::vector<Exp*> returns;

public:
    ReturnStatement();
    ~ReturnStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Statement* clone();

    // Accept a visitor to this RTL
    virtual bool accept(StmtVisitor* visitor);

    // print
    virtual void print(std::ostream& os = std::cout, bool withDF = false);

    // From SSA form
    virtual void fromSSAform(igraph& igm);

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

    // simplify all the uses/defs in this RTL
    virtual void simplify();

    // general search
    virtual bool search(Exp*, Exp*&);

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(Exp* search, Exp* replace);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.    
    virtual bool searchAll(Exp* search, std::list<Exp*> &result);

    // returns true if this statement uses the given expression
    virtual bool usesExp(Exp *e);

    // Adds (inserts) all locations (registers or memory) used by this
    // statement
    virtual void addUsedLocs(LocationSet& used);

    virtual void fixCallRefs();

    // Subscript all occurrences of e with definition def (except for top level
    // of LHS of assignment)
    virtual void subscriptVar(Exp* e, Statement* def);

    virtual void doReplaceRef(Exp* from, Exp* to);

    int getNumBytesPopped() { return nBytesPopped; }
    void setNumBytesPopped(int n) { nBytesPopped = n; }

    int getNumReturns() { return returns.size(); }
    Exp *getReturnExp(int n) { return returns[n]; }
    void setReturnExp(int n, Exp *e) { returns[n] = e; }
    void setSigArguments();   // Set returns based on signature
    void removeReturn(int n);
    void addReturn(Exp *e);

};  // class ReturnStatement


/*==============================================================================
 * BoolStatement represents "setCC" type instructions, where some destination is
 * set (to 1 or 0) depending on the condition codes. It has a condition
 * Exp, similar to the BranchStatement class.
 * *==========================================================================*/
class BoolStatement: public Statement {
    BRANCH_TYPE jtCond;            // the condition for setting true
    Exp* pCond;                    // Exp representation of the high level
                                   // condition: e.g. r[8] == 5
    bool bFloat;                   // True if condition uses floating point CC
    Exp* pDest;                    // The location assigned (with 0 or 1)
    int  size;                     // The size of the dest
public:
    BoolStatement(int size);
    virtual ~BoolStatement();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual Statement* clone();

    // Accept a visitor to this RTL
    virtual bool accept(StmtVisitor* visitor);

    // Set and return the BRANCH_TYPE of this scond as well as whether the
    // floating point condition codes are used.
    void setCondType(BRANCH_TYPE cond, bool usesFloat = false);
    BRANCH_TYPE getCond(){return jtCond;}
    bool isFloat(){return bFloat;}
    void setFloat(bool b) { bFloat = b; }

    // Set and return the Exp representing the HL condition
    Exp* getCondExpr();
    void setCondExpr(Exp* pss);
    // As above, no delete (for subscripting)
    void setCondExprND(Exp* e) { pCond = e; }

    Exp* getDest() {return pDest;}  // Return the destination of the set
    void setDest(std::list<Statement*>* stmts);
    int getSize() {return size;}    // Return the size of the assignment

    void makeSigned();

    virtual void print(std::ostream& os = std::cout, bool withDF = false);

#if 0
    // Used for type analysis. Stores type information that
    // can be gathered from the RTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);       
#endif

    // code generation
    virtual void generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

    // simplify all the uses/defs in this RTL
    virtual void simplify();

    // Statement functions
    virtual void addUsedLocs(LocationSet& used);
    virtual void fixCallRefs();
    virtual void subscriptVar(Exp* e, Statement* def);
    //virtual void getDeadStatements(StatementSet &dead);
    virtual bool isDefinition() { return true; }
    virtual void getDefinitions(LocationSet &def);
    virtual Exp* getLeft() { return getDest(); }
    virtual Type* getLeftType();
    virtual Exp* getRight() { return getCondExpr(); }
    virtual bool usesExp(Exp *e);
    virtual void print(std::ostream &os) { print(os, false); }
    virtual void processConstants(Prog *prog);
    virtual bool search(Exp *search, Exp *&result);
    virtual bool searchAll(Exp* search, std::list<Exp*>& result);
    virtual bool searchAndReplace(Exp *search, Exp *replace);
    virtual Type* updateType(Exp *e, Type *curType);
    virtual void doReplaceRef(Exp* from, Exp* to);
    // from SSA form
    virtual void fromSSAform(igraph& ig);

};

/* 
 * The StmtVisitor class is used to iterate over all stmts in a basic 
 * block. It contains methods for each kind of Statement found in an
 * RTL and can be used to eliminate switch statements.
 */
class StmtVisitor {
private:
    // the enclosing basic block
    PBB pBB;

public:
    StmtVisitor() { pBB = NULL; }
    virtual ~StmtVisitor() { }

    // allows the container being iteratorated over to identify itself
    PBB getBasicBlock() { return pBB; }
    void setBasicBlock(PBB bb) { pBB = bb; }

    // visitor functions, 
    // returns true to continue iteratoring the container
    virtual bool visit(RTL *rtl) = 0;
    virtual bool visit(Assign *stmt) = 0;
    virtual bool visit(GotoStatement *stmt) = 0;
    virtual bool visit(BranchStatement *stmt) = 0;
    virtual bool visit(CaseStatement *stmt) = 0;
    virtual bool visit(CallStatement *stmt) = 0;
    virtual bool visit(ReturnStatement *stmt) = 0;
    virtual bool visit(BoolStatement *stmt) = 0;
};


#endif // __STATEMENT_H__
