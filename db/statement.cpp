/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       statement.cpp
 * OVERVIEW:   Implementation of the Statement and related classes.
 *             (Was dataflow.cpp)
 *============================================================================*/

/*
 * $Revision$
 * 03 Jul 02 - Trent: Created
 * 09 Jan 03 - Mike: Untabbed, reformatted
 * 03 Feb 03 - Mike: cached dataflow (uses and usedBy) (since reversed)
 * 25 Jul 03 - Mike: dataflow.cpp, hrtl.cpp -> statement.cpp
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#include <iomanip>          // For setfill etc
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include "statement.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "boomerang.h"
#include "rtl.h"            // For debugging code
#include "util.h"
#include "signature.h"
#include "visitor.h"
#include <sstream>

extern char debug_buffer[];      // For prints functions

void Statement::setProc(UserProc *p)
{
    proc = p;
    LocationSet exps;
    addUsedLocs(exps);
    LocationSet defs;
    getDefinitions(defs);
    exps.makeUnion(defs);
    LocationSet::iterator ll;
    for (ll = exps.begin(); ll != exps.end(); ll++) {
        Location *l = dynamic_cast<Location*>(*ll);
        if (l) {
            l->setProc(p);
        }
    }
}

// replace a use in this statement
bool Statement::replaceRef(Statement *def) {
    Exp* lhs = def->getLeft();
    Exp* rhs = def->getRight();
    assert(lhs);
    assert(rhs);
    // "Wrap" the LHS in a RefExp
    // This was so that it matches with the thing it is replacing.
    // Example: 42: r28 := r28{14}-4 into m[r28-24] := m[r28{42}] + ...
    // The r28 needs to be subscripted with {42} to match the thing on
    // the RHS that is being substituted into. (It also makes sure it
    // never matches the other r28, which should really be r28{0}).
    Unary* re;
    re = new RefExp(lhs, def);

    // do the replacement
    bool convert = doReplaceRef(re, rhs);

    // Careful: don't delete re while lhs is still a part of it!
    // Else, will delete lhs, which is still a part of def!
    re->setSubExp1ND(NULL);
    //delete re;
    return convert;
}

// Check the liveout set for interferences
// Examples:  r[24]{3} and r[24]{5} both live at same time,
// or m[r[28]{3}] and m[r[28]{3}]{2}
static int nextVarNum = 0;
void insertInterference(igraph& ig, Exp* e) {
    igraph::iterator it = ig.find(e);
    if (it == ig.end()) {
        // We will be inserting a new element
		const std::pair<Exp*, int> temp(e, ++nextVarNum);
        ig.insert(temp);
	}
    // else it is already in the map: no need to do anything
}

bool Statement::mayAlias(Exp *e1, Exp *e2, int size) { 
    if (*e1 == *e2) return true;
    // Pass the expressions both ways. Saves checking things like
    // m[exp] vs m[exp+K] and m[exp+K] vs m[exp] explicitly (only need to
    // check one of these cases)
    bool b =  (calcMayAlias(e1, e2, size) && calcMayAlias(e2, e1, size)); 
    if (b && VERBOSE) {
        LOG << "May alias: " << e1 << " and " << e2 << " size " << size
          << "\n";
    }
    return b;
}

// returns true if e1 may alias e2
bool Statement::calcMayAlias(Exp *e1, Exp *e2, int size) {
    // currently only considers memory aliasing..
    if (!e1->isMemOf() || !e2->isMemOf()) {
        return false;
    }
    Exp *e1a = e1->getSubExp1();
    Exp *e2a = e2->getSubExp1();
    // constant memory accesses
    if (e1a->isIntConst() && 
        e2a->isIntConst()) {
        ADDRESS a1 = ((Const*)e1a)->getAddr();
        ADDRESS a2 = ((Const*)e2a)->getAddr();
        int diff = a1 - a2;
        if (diff < 0) diff = -diff;
        if (diff*8 >= size) return false;
    }
    // same left op constant memory accesses
    if (
      e1a->getArity() == 2 &&
      e1a->getOper() == e2a->getOper() &&
      e1a->getSubExp2()->isIntConst() &&
      e2a->getSubExp2()->isIntConst() &&
      *e1a->getSubExp1() == *e2a->getSubExp1()) {
        int i1 = ((Const*)e1a->getSubExp2())->getInt();
        int i2 = ((Const*)e2a->getSubExp2())->getInt();
        int diff = i1 - i2;
        if (diff < 0) diff = -diff;
        if (diff*8 >= size) return false;
    }
    // [left] vs [left +/- constant] memory accesses
    if (
      (e2a->getOper() == opPlus || e2a->getOper() == opMinus) &&
      *e1a == *e2a->getSubExp1() &&
      e2a->getSubExp2()->isIntConst()) {
        int i1 = 0;
        int i2 = ((Const*)e2a->getSubExp2())->getInt();
        int diff = i1 - i2;
        if (diff < 0) diff = -diff;
        if (diff*8 >= size) return false;
    }
    // Don't need [left +/- constant ] vs [left] because called twice with
    // args reversed
    return true;
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for Statement* etc
 *                  Just makes it easier to use e.g. std::cerr << myStmtStar
 * PARAMETERS:      os: output stream to send to
 *                  p: ptr to Statement to print to the stream
 * RETURNS:         copy of os (for concatenation)
 *============================================================================*/
std::ostream& operator<<(std::ostream& os, Statement* s) {
    if (s == NULL) {os << "NULL "; return os;}
    s->print(os, true);
    return os;
}

bool Statement::isOrdinaryAssign() {
    if (kind != STMT_ASSIGN)
        return false;
    OPER op = ((Assign*)this)->getRight()->getOper();
    if (op == opFlagCall) return false;
    if (op == opPhi) return false;
    return true;
}

bool Statement::isFlagAssgn() {
    if (kind != STMT_ASSIGN)
        return false;
    OPER op = ((Assign*)this)->getRight()->getOper();
    return (op == opFlagCall);
}

    
bool Statement::isPhi() {
    if (kind != STMT_ASSIGN)
        return false;
    Exp* rhs = ((Assign*)this)->getRight();
    return /*rhs &&*/  rhs->isPhi();
}
    
char* Statement::prints() {
    std::ostringstream ost;
    print(ost, true);
    strncpy(debug_buffer, ost.str().c_str(), 199);
    debug_buffer[199] = '\0';
    return debug_buffer;
}

/* This function is designed to find basic flag calls, plus in addition
// two variations seen with Pentium FP code. These variations involve
// ANDing and/or XORing with constants. So it should return true for these
// values of e:
 ADDFLAGS(...)
 SETFFLAGS(...) & 0x45
 (SETFFLAGS(...) & 0x45) ^ 0x40
*/
bool hasSetFlags(Exp* e) {
    if (e->isFlagCall()) return true;
    OPER op = e->getOper();
    if (op != opBitAnd && op != opBitXor) return false;
    Exp* left  = ((Binary*)e)->getSubExp1();
    Exp* right = ((Binary*)e)->getSubExp2();
    if (!right->isIntConst()) return false;
    if (left->isFlagCall()) return true;
    op = left->getOper();
    if (op != opBitAnd && op != opBitXor) return false;
    right = ((Binary*)left)->getSubExp2();
    left  = ((Binary*)left)->getSubExp1();
    if (!right->isIntConst()) return false;
    return left->isFlagCall();
}

// exclude: a set of statements not to propagate from
// memDepth: the max level of expressions to propagate FROM
// toDepth: the exact level that we will propagate TO (unless toDepth == -1)
// Reasoning is as follows: you can't safely propagate and memory expression
// until all its subexpressions (expressions at lower memory depths) are
// propagated. Examples:
// 1: r28 = r28-4
// 2: m[m[r28]+4] = 10
// 3: r28 = r28-4
// 4: r25 = m[m[r28]+4]   // Appears defined in 2 if don't consider FROM depths
// 5: r24 = r28
// 6: m[r24] = r[28]
// 6: m[r25] = 4
// 7: m[m[m[r24]]+m[r25]] = 99
// Ordinarily would subst m[r24] first, at level 2... ugh, can't think of an
// example where it would matter.

// Return true if an indirect call statement is converted to direct
bool Statement::propagateTo(int memDepth, StatementSet& exclude, int toDepth) 
{
#if 0       // If don't propagate into flag assigns, some converting to locals
            // doesn't happen, and errors occur
    // don't propagate to flag assigns
    if (isFlagAssgn())
        return;
#endif
    // don't propagate into temp definitions (? why? Can this ever happen?)
    if (isAssign() && getLeft()->isTemp())
        return false;
    bool change;
    bool convert = false;
    int changes = 0;
    // Repeat substituting into this statement while there is a single reference
    // component in it
    // But all RefExps will have just one component. Maybe calls (later) will
    // have more than one ref
    // Example: y := a{2,3} + b{4} + c{0}
    // can substitute b{4} into this, but not a. Can't do c either, since there
    // is no definition (it's a parameter).
    do {
        LocationSet exps;
        addUsedLocs(exps);
        LocationSet::iterator ll;
        change = false;
        for (ll = exps.begin(); ll != exps.end(); ll++) {
            Exp* m = *ll;
            if (toDepth != -1 && m->getMemDepth() != toDepth)
                continue;
            LocationSet refs;
            m->addUsedLocs(refs);
            LocationSet::iterator rl;
            for (rl = refs.begin(); rl != refs.end(); rl++) {
                Exp* e = *rl;
                if (!e->getNumRefs() == 1) continue;
                // Can propagate TO this (if memory depths are suitable)
                Statement* def;
                def = ((RefExp*)e)->getRef();
                if (def == NULL)
                    // Can't propagate statement "0"
                    continue;
                if (def == this)
                    // Don't propagate to self! Can happen with %pc's
                    continue;
                if (def->isNullStatement())
                    // Don't propagate a null statement! Can happen with %pc's
                    // (this would have no effect, and would infinitely loop)
                    continue;
                // Don't propagate from statements in the exclude list
                if (exclude.exists(def)) continue;
                if (def->isPhi())
                    // Don't propagate phi statements!
                    continue;
                if (def->isCall())
                    continue;
                if (def->isBool())
                    continue;
#if 1   // Sorry, I don't believe prop into branches is wrong... MVE
        // By not propagating into branches, we get memory locations not
        // converted to locals, for example (e.g. test/source/csp.c)
         
                if (isBranch()) {
                    Exp* defRight = def->getRight();
                    // Propagating to a branch often doesn't give good results,
                    // unless propagating flags
                    // Special case for Pentium: allow prop of
                    // r12 := SETFFLAGS(...) (fflags stored via register AH)
                    if (!hasSetFlags(defRight))
                        // Allow propagation of constants
                        if (defRight->isIntConst() || defRight->isFltConst())
                            if (VERBOSE) LOG << "Allowing prop. into "
                                "branch (1) of " << def << "\n";
                            else
                                ;
                        // ?? Also allow any assignments to temps or assignment
                        // of anything to anything subscripted. Trent:
                        // was the latter meant to be anything NOT subscripted?
                        else if (defRight->getOper() != opSubscript &&
                            !def->getLeft()->isTemp())
                            continue;
                        else
                            if (VERBOSE) LOG << "Allowing prop. into "
                                "branch (2) of " << def << "\n";
                }
#endif
                if (def->getLeft()->getType() && 
                    def->getLeft()->getType()->isArray()) {
                    // Assigning to an array, don't propagate
                    continue;
                }
                change = doPropagateTo(memDepth, def, convert);
            }
        }
    } while (change && ++changes < 20);
    // Simplify is very costly, especially for calls. I hope that doing one
    // simplify at the end will not affect any result...
    simplify();
    return convert;
}

// Parameter convert is set true if an indirect call is converted to direct
// Return true if a change made
bool Statement::doPropagateTo(int memDepth, Statement* def, bool& convert) {
    // Check the depth of the definition (an assignment)
    // This checks the depth for the left and right sides, and
    // gives the max for both. Example: can't propagate
    // tmp := m[x] to foo := tmp if memDepth == 0
    Assign* assignDef = dynamic_cast<Assign*>(def);
    if (assignDef) {
        int depth = assignDef->getMemDepth();
        if (depth > memDepth)
            return false;
    }

    // Respect the -p N switch
    if (Boomerang::get()->numToPropagate >= 0) {
        if (Boomerang::get()->numToPropagate == 0) return false;
            Boomerang::get()->numToPropagate--;
    }

    if (VERBOSE) {
        LOG << "Propagating " << def << "\n"
            << "       into " << this << "\n";
    }
    convert |= replaceRef(def);
    // simplify is costly... done once above
    // simplify();
    if (VERBOSE) {
        LOG << "     result " << this << "\n\n";
    }
    return true;
}

bool Statement::operator==(Statement& other) {
    // When do we need this?
    assert(0);
#if 0
    Assign* ae1 = dynamic_cast<Assign*>(this);
    Assign* ae2 = dynamic_cast<Assign*>(&other);
    assert(ae1);
    assert(ae2);
    return *ae1 == *ae2;
#else
    return false;
#endif
}

bool Statement::isNullStatement() {
    if (kind != STMT_ASSIGN) return false;
    Exp* right = ((Assign*)this)->getRight();
    if (right->isSubscript()) {
        // Must refer to self to be null
        return this == ((RefExp*)right)->getRef();
    }
    else
        // Null if left == right
        return *((Assign*)this)->getLeft() == *right;
}

bool Statement::isFpush() {
    if (kind != STMT_ASSIGN) return false;
    return ((Assign*)this)->getRight()->getOper() == opFpush;
}
bool Statement::isFpop() {
    if (kind != STMT_ASSIGN) return false;
    return ((Assign*)this)->getRight()->getOper() == opFpop;
}


/*
 * This code was in hrtl.cpp
 * Classes derived from Statement
 */


#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

/******************************************************************************
 * GotoStatement methods
 *****************************************************************************/

/*==============================================================================
 * FUNCTION:        GotoStatement::GotoStatement
 * OVERVIEW:        Constructor.
 * PARAMETERS:      listStmt: a list of Statements (not the same as an RTL)
 *                    to serve as the initial list of statements
 * RETURNS:         N/a
 *============================================================================*/
GotoStatement::GotoStatement()
  : pDest(NULL), m_isComputed(false) {
    kind = STMT_GOTO;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::GotoStatement
 * OVERVIEW:        Construct a jump to a fixed address
 * PARAMETERS:      uDest: native address of destination
 * RETURNS:         N/a
 *============================================================================*/
GotoStatement::GotoStatement(ADDRESS uDest) : m_isComputed(false) {
    kind = STMT_GOTO;
    pDest = new Const(uDest);
}

/*==============================================================================
 * FUNCTION:        GotoStatement::~GotoStatement
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
GotoStatement::~GotoStatement() {
    if (pDest) ;//delete pDest;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::getFixedDest
 * OVERVIEW:        Get the fixed destination of this CTI. Assumes destination
 *                  simplication has already been done so that a fixed dest will
 *                  be of the Exp form:
 *                     opIntConst dest
 * PARAMETERS:      <none>
 * RETURNS:         Fixed dest or NO_ADDRESS if there isn't one
 *============================================================================*/
ADDRESS GotoStatement::getFixedDest() {
    if (pDest->getOper() != opIntConst) return NO_ADDRESS;
    return ((Const*)pDest)->getAddr();
}

/*==============================================================================
 * FUNCTION:        GotoStatement::setDest
 * OVERVIEW:        Set the destination of this jump to be a given expression.
 * PARAMETERS:      addr - the new fixed address
 * RETURNS:         Nothing
 *============================================================================*/
void GotoStatement::setDest(Exp* pd) {
    if (pDest != NULL)
        ;//delete pDest;
    pDest = pd;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::setDest
 * OVERVIEW:        Set the destination of this jump to be a given fixed address.
 * PARAMETERS:      addr - the new fixed address
 * RETURNS:         <nothing>
 *============================================================================*/
void GotoStatement::setDest(ADDRESS addr) {
// This fails in FrontSparcTest, do you really want it to Mike? -trent
//  assert(addr >= prog.limitTextLow && addr < prog.limitTextHigh);
    // Delete the old destination if there is one
    if (pDest != NULL)
        ;//delete pDest;

    pDest = new Const(addr);
}

/*==============================================================================
 * FUNCTION:        GotoStatement::getDest
 * OVERVIEW:        Returns the destination of this CTI.
 * PARAMETERS:      None
 * RETURNS:         Pointer to the SS representing the dest of this jump
 *============================================================================*/
Exp* GotoStatement::getDest() {
    return pDest;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::adjustFixedDest
 * OVERVIEW:        Adjust the destination of this CTI by a given amount. Causes
 *                  an error is this destination is not a fixed destination
 *                  (i.e. a constant offset).
 * PARAMETERS:      delta - the amount to add to the destination (can be
 *                  negative)
 * RETURNS:         <nothing>
 *============================================================================*/
void GotoStatement::adjustFixedDest(int delta) {
    // Ensure that the destination is fixed.
    if (pDest == 0 || pDest->getOper() != opIntConst)
        LOG << "Can't adjust destination of non-static CTI\n";

    ADDRESS dest = ((Const*)pDest)->getAddr();
    ((Const*)pDest)->setAddr(dest + delta);
}

bool GotoStatement::search(Exp* search, Exp*& result) {
    if (pDest)
        return pDest->search(search, result);
    return false;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         True if any change
 *============================================================================*/
bool GotoStatement::searchAndReplace(Exp* search, Exp* replace) {
    bool change = false;
    if (pDest) {
        pDest = pDest->searchReplaceAll(search, replace, change);
    }
    return change;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool GotoStatement::searchAll(Exp* search, std::list<Exp*> &result) {
    if (pDest)  return pDest->searchAll(search, result);
    return false;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::print
 * OVERVIEW:        Display a text reprentation of this RTL to the given stream
 * NOTE:            Usually called from RTL::print, in which case the first 9
 *                    chars of the print have already been output to os
 * PARAMETERS:      os: stream to write to
 * RETURNS:         Nothing
 *============================================================================*/
void GotoStatement::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::setw(4) << std::dec << number << " ";
    os << "GOTO ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (pDest->getOper() != opIntConst)
         pDest->print(os);
    else
        os << "0x" << std::hex << getFixedDest();
}

/*==============================================================================
 * FUNCTION:      GotoStatement::setIsComputed
 * OVERVIEW:      Sets the fact that this call is computed.
 * NOTE:          This should really be removed, once CaseStatement and
 *                  HLNwayCall are implemented properly
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
void GotoStatement::setIsComputed(bool b) {
    m_isComputed = b;
}

/*==============================================================================
 * FUNCTION:      GotoStatement::isComputed
 * OVERVIEW:      Returns whether or not this call is computed.
 * NOTE:          This should really be removed, once CaseStatement and HLNwayCall
 *                  are implemented properly
 * PARAMETERS:    <none>
 * RETURNS:       this call is computed
 *============================================================================*/
bool GotoStatement::isComputed() {
    return m_isComputed;
}

/*==============================================================================
 * FUNCTION:        GotoStatement::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new Statement, a clone of this GotoStatement
 *============================================================================*/
Statement* GotoStatement::clone() {
    GotoStatement* ret = new GotoStatement();
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    // Statement members
    ret->pbb = pbb;
    ret->proc = proc;
    ret->number = number;
    return ret;
}

// visit this Statement in the RTL
bool GotoStatement::accept(StmtVisitor* visitor) {
    return visitor->visit(this);
}

void GotoStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    // dont generate any code for jumps, they will be handled by the BB
}

void GotoStatement::simplify() {
    if (isComputed()) {
        pDest = pDest->simplifyArith();
        pDest = pDest->simplify();
    }
}

/**********************************
 * BranchStatement methods
 **********************************/

/*==============================================================================
 * FUNCTION:        BranchStatement::BranchStatement
 * OVERVIEW:        Constructor.
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
BranchStatement::BranchStatement() : jtCond((BRANCH_TYPE)0), pCond(NULL),
  bFloat(false) {
    kind = STMT_BRANCH;
}

/*==============================================================================
 * FUNCTION:        BranchStatement::~BranchStatement
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
BranchStatement::~BranchStatement() {
    if (pCond)
        ;//delete pCond;
}

/*==============================================================================
 * FUNCTION:        BranchStatement::setCondType
 * OVERVIEW:        Sets the BRANCH_TYPE of this jcond as well as the flag
 *                  indicating whether or not the floating point condition codes
 *                  are used.
 * PARAMETERS:      cond - the BRANCH_TYPE
 *                  usesFloat - this condional jump checks the floating point
 *                    condition codes
 * RETURNS:         a semantic string
 *============================================================================*/
void BranchStatement::setCondType(BRANCH_TYPE cond, bool usesFloat /*= false*/) {
    jtCond = cond;
    bFloat = usesFloat;

    // set pCond to a high level representation of this type
    Exp* p = NULL;
#if 0
    switch(cond) {
        case BRANCH_JE:
            p = new Terminal(opZF);
            break;
        case BRANCH_JNE:
            p = new Unary(opNot, new Terminal(opZF));
            break;
        case BRANCH_JSL:
            // N xor V
            p = new Binary(opNotEqual, new Terminal(opNF), new Terminal(opOF));
            break;
        case BRANCH_JSLE:
            // Z or (N xor V)
            p = new Binary(opOr,
                new Terminal(opZF),
                new Binary(opNotEqual, new Terminal(opNF), new Terminal(opOF)));
            break;
        case BRANCH_JSGE:
            // not (N xor V) same as (N == V)
            p = new Binary(opEquals, new Terminal(opNF), new Terminal(opOF));
            break;
        case BRANCH_JSG:
            // not (Z or (N xor V))
            p = new Unary(opNot,
                new Binary(opOr,
                    new Terminal(opZF),
                    new Binary(opNotEqual,
                        new Terminal(opNF), new Terminal(opOF))));
            break;
        case BRANCH_JUL:
            // C
            p = new Terminal(opCF);
            break;
        case BRANCH_JULE:
            // C or Z
            p = new Binary(opOr, new Terminal(opCF), 
                                 new Terminal(opZF));
            break;
        case BRANCH_JUGE:
            // not C
            p = new Unary(opNot, new Terminal(opCF));
            break;
        case BRANCH_JUG:
            // not (C or Z)
            p = new Unary(opNot,
                new Binary(opOr,
                    new Terminal(opCF),
                    new Terminal(opZF)));
            break;
        case BRANCH_JMI:
            // N
            p = new Terminal(opNF);
            break;
        case BRANCH_JPOS:
            // not N
            p = new Unary(opNot, new Terminal(opNF));
            break;
        case BRANCH_JOF:
            // V
            p = new Terminal(opOF);
            break;
        case BRANCH_JNOF:
            // not V
            p = new Unary(opNot, new Terminal(opOF));
            break;
        case BRANCH_JPAR:
            // Can't handle (could happen as a result of a failure of Pentium
            // floating point analysis)
            assert(false);
            break;
    }
#else
    p = new Terminal(usesFloat ? opFflags : opFlags);
#endif
    assert(p);
    setCondExpr(p);
}

/*==============================================================================
 * FUNCTION:        BranchStatement::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void BranchStatement::makeSigned() {
    // Make this into a signed branch
    switch (jtCond)
    {
        case BRANCH_JUL : jtCond = BRANCH_JSL;  break;
        case BRANCH_JULE: jtCond = BRANCH_JSLE; break;
        case BRANCH_JUGE: jtCond = BRANCH_JSGE; break;
        case BRANCH_JUG : jtCond = BRANCH_JSG;  break;
        default:
            // Do nothing for other cases
            break;
    }
}

/*==============================================================================
 * FUNCTION:        BranchStatement::getCondExpr
 * OVERVIEW:        Return the SemStr expression containing the HL condition.
 * PARAMETERS:      <none>
 * RETURNS:         ptr to an expression
 *============================================================================*/
Exp* BranchStatement::getCondExpr() {
    return pCond;
}

/*==============================================================================
 * FUNCTION:        BranchStatement::setCondExpr
 * OVERVIEW:        Set the SemStr expression containing the HL condition.
 * PARAMETERS:      Pointer to Exp to set
 * RETURNS:         <nothing>
 *============================================================================*/
void BranchStatement::setCondExpr(Exp* e) {
    if (pCond) ;//delete pCond;
    pCond = e;
}

bool BranchStatement::search(Exp* search, Exp*& result) {
    if (pCond) return pCond->search(search, result);
    result = NULL;
    return false;
}

/*==============================================================================
 * FUNCTION:        BranchStatement::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         True if any change
 *============================================================================*/
bool BranchStatement::searchAndReplace(Exp* search, Exp* replace) {
    GotoStatement::searchAndReplace(search, replace);
    bool change = false;
    if (pCond)
        pCond = pCond->searchReplaceAll(search, replace, change);
    return change;
}

// update type for expression
Type *BranchStatement::updateType(Exp *e, Type *curType) {
    if (jtCond == BRANCH_JUGE || jtCond == BRANCH_JULE ||
        jtCond == BRANCH_JUG || jtCond == BRANCH_JUL && 
        curType->isInteger()) {
        ((IntegerType*)curType)->setSigned(false);
    }
    return curType;
}

// Convert from SSA form
void BranchStatement::fromSSAform(igraph& ig) {
    if (pCond)
        pCond = pCond->fromSSA(ig); 
}

/*==============================================================================
 * FUNCTION:        BranchStatement::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool BranchStatement::searchAll(Exp* search, std::list<Exp*> &result) {
    if (pCond) return pCond->searchAll(search, result);
    return false;
}


/*==============================================================================
 * FUNCTION:        BranchStatement::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         Nothing
 *============================================================================*/
void BranchStatement::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::setw(4) << std::dec << number << " ";
    os << "BRANCH ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (!pDest->isIntConst())
        os << pDest;
    else {
        // Really we'd like to display the destination label here...
        os << "0x" << std::hex << getFixedDest();
    }
    os << ", condition ";
    switch (jtCond) {
        case BRANCH_JE:    os << "equals"; break;
        case BRANCH_JNE:   os << "not equals"; break;
        case BRANCH_JSL:   os << "signed less"; break;
        case BRANCH_JSLE:  os << "signed less or equals"; break;
        case BRANCH_JSGE:  os << "signed greater or equals"; break;
        case BRANCH_JSG:   os << "signed greater"; break;
        case BRANCH_JUL:   os << "unsigned less"; break;
        case BRANCH_JULE:  os << "unsigned less or equals"; break;
        case BRANCH_JUGE:  os << "unsigned greater or equals"; break;
        case BRANCH_JUG:   os << "unsigned greater"; break;
        case BRANCH_JMI:   os << "minus"; break;
        case BRANCH_JPOS:  os << "plus"; break;
        case BRANCH_JOF:   os << "overflow"; break;
        case BRANCH_JNOF:  os << "no overflow"; break;
        case BRANCH_JPAR:  os << "parity"; break;
    }
    if (bFloat) os << " float";
    os << std::endl;
    if (pCond) {
        os << "High level: " << pCond;
    }
}

/*==============================================================================
 * FUNCTION:        BranchStatement::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new Statement, a clone of this BranchStatement
 *============================================================================*/
Statement* BranchStatement::clone() {
    BranchStatement* ret = new BranchStatement();
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    ret->jtCond = jtCond;
    if (pCond) ret->pCond = pCond->clone();
    else ret->pCond = NULL;
    ret->m_isComputed = m_isComputed;
    ret->bFloat = bFloat;
    // Statement members
    ret->pbb = pbb;
    ret->proc = proc;
    ret->number = number;
    return ret;
}

// visit this rtl
bool BranchStatement::accept(StmtVisitor* visitor) {
    return visitor->visit(this);
}

void BranchStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    // dont generate any code for jconds, they will be handled by the bb
}

bool BranchStatement::usesExp(Exp *e) {
    Exp *tmp;
    return pCond && pCond->search(e, tmp);
}

// process any constants in the statement
void BranchStatement::processConstants(Prog *prog) {
}

bool BranchStatement::doReplaceRef(Exp* from, Exp* to) {
    bool change;
    assert(pCond);
    pCond = pCond->searchReplaceAll(from, to, change);
    simplify();
    return false;
}


// Common to BranchStatement and BoolStatement
// Return true if this is now a floating point Branch
bool condToRelational(Exp*& pCond, BRANCH_TYPE jtCond) {
    pCond = pCond->simplifyArith()->simplify();

    std::stringstream os;
    pCond->print(os);
    std::string s = os.str();

    OPER condOp = pCond->getOper();
    if (condOp == opFlagCall &&
          !strncmp(((Const*)pCond->getSubExp1())->getStr(),
          "SUBFLAGS", 8)) {
        OPER op = opWild;
        switch (jtCond) {
            case BRANCH_JE:    op = opEquals; break;
            case BRANCH_JNE:   op = opNotEqual; break;
            case BRANCH_JSL:   op = opLess; break;
            case BRANCH_JSLE:  op = opLessEq; break;
            case BRANCH_JSGE:  op = opGtrEq; break;
            case BRANCH_JSG:   op = opGtr; break;
            case BRANCH_JUL:   op = opLessUns; break;
            case BRANCH_JULE:  op = opLessEqUns; break;
            case BRANCH_JUGE:  op = opGtrEqUns; break;
            case BRANCH_JUG:   op = opGtrUns; break;
            case BRANCH_JMI:
                pCond = new Binary(opLess,
                    pCond->getSubExp2()->getSubExp2()->getSubExp2()
                        ->getSubExp1()->clone(), new Const(0));
                break;
            case BRANCH_JPOS:
                pCond = new Binary(opGtrEq,
                    pCond->getSubExp2()->getSubExp2()->getSubExp2()
                        ->getSubExp1()->clone(), new Const(0));
                break;
            case BRANCH_JOF:
            case BRANCH_JNOF:
            case BRANCH_JPAR:
                break;
        }
        if (op != opWild) {
            pCond = new Binary(op,
                pCond->getSubExp2()->getSubExp1()->clone(),
                pCond->getSubExp2()->getSubExp2()->getSubExp1()
                    ->clone());
        }
    }
    else if (condOp == opFlagCall && 
          !strncmp(((Const*)pCond->getSubExp1())->getStr(), 
          "LOGICALFLAGS", 12)) {
        // Exp *e = pCond;
        OPER op = opWild;
        switch (jtCond) {
            case BRANCH_JE:   op = opEquals; break;
            case BRANCH_JNE:  op = opNotEqual; break;
            case BRANCH_JMI:  op = opLess; break;
            case BRANCH_JPOS: op = opGtrEq; break;
            case BRANCH_JSL:  op = opLess; break;
            case BRANCH_JSLE: op = opLessEq; break;
            case BRANCH_JSGE: op = opGtrEq; break;
            case BRANCH_JSG:  op = opGtr; break;
            case BRANCH_JUL:  op = opLessUns; break;
            case BRANCH_JULE: op = opLessEqUns; break;
            case BRANCH_JUGE: op = opGtrEqUns; break;
            case BRANCH_JUG:  op = opGtrUns; break;
            default:
                break;
        }
        if (op != opWild) {
            pCond = new Binary(op,
                pCond->getSubExp2()->getSubExp1()->clone(),
                new Const(0));
        }
    }
    else if (condOp == opFlagCall && 
          !strncmp(((Const*)pCond->getSubExp1())->getStr(), 
          "SETFFLAGS", 9)) {
        // Exp *e = pCond;
        OPER op = opWild;
        switch (jtCond) {
            case BRANCH_JE:   op = opEquals; break;
            case BRANCH_JNE:  op = opNotEqual; break;
            case BRANCH_JMI:  op = opLess; break;
            case BRANCH_JPOS: op = opGtrEq; break;
            case BRANCH_JSL:  op = opLess; break;
            case BRANCH_JSLE: op = opLessEq; break;
            case BRANCH_JSGE: op = opGtrEq; break;
            case BRANCH_JSG:  op = opGtr; break;
            default:
                break;
        }
        if (op != opWild) {
            pCond = new Binary(op,
                pCond->getSubExp2()->getSubExp1()->clone(),
                pCond->getSubExp2()->getSubExp2()->getSubExp1()
                    ->clone());
        }
    }
    // ICK! This is all PENTIUM SPECIFIC... needs to go somewhere else
    // Might be of the form (SETFFLAGS(...) & MASK) RELOP INTCONST
    // where MASK could be a combination of 1, 4, and 40, and
    // relop could be == or ~=
    // There could also be an XOR 40h after the AND
    // %fflags = 0..0.0 00 >
    // %fflags = 0..0.1 01 <
    // %fflags = 1..0.0 40 =
    // %fflags = 1..1.1 45 not comparable
    // Example: (SETTFLAGS(...) & 1) ~= 0
    // left = SETFFLAGS(...) & 1
    // left1 = SETFFLAGS(...) left2 = int 1, k = 0, mask = 1
    else if (condOp == opEquals || condOp == opNotEqual) {
        Exp* left =  ((Binary*)pCond)->getSubExp1();
        Exp* right = ((Binary*)pCond)->getSubExp2();
        bool hasXor40 = false;
        if (left->getOper() == opBitXor && right->isIntConst()) {
            Exp* r2 = ((Binary*)left)->getSubExp2();
            if (r2->isIntConst()) {
                int k2 = ((Const*)r2)->getInt();
                if (k2 == 0x40) {
                    hasXor40 = true;
                    left = ((Binary*)left)->getSubExp1();
                }
            }
        }
        if (left->getOper() == opBitAnd && right->isIntConst()) {
            Exp* left1 = ((Binary*)left)->getSubExp1();
            Exp* left2 = ((Binary*)left)->getSubExp2();
            int k = ((Const*)right)->getInt();
            // Only interested in 40, 1
            k &= 0x41;
            if (left1->getOper() == opFlagCall &&
                  left2->isIntConst()) {
                int mask = ((Const*)left2)->getInt();
                // Only interested in 1, 40
                mask &= 0x41;
                OPER op = opWild;
                if (hasXor40) {
                    assert(k == 0);
                    op = condOp;
                } else {
                    switch (mask) {
                        case 1:
                            if (condOp == opEquals && k == 0 ||
                                condOp == opNotEqual && k == 1)
                                    op = opGtrEq;
                            else
                                    op = opLess;
                            break;
                        case 0x40:
                            if (condOp == opEquals && k == 0 ||
                                condOp == opNotEqual && k == 0x40)
                                    op = opNotEqual;
                            else
                                    op = opEquals;
                            break;
                        case 0x41:
                            switch (k) {
                                case 0:
                                    if (condOp == opEquals) op = opGtr;
                                    else op = opLessEq;
                                    break;
                                case 1:
                                    if (condOp == opEquals) op = opLess;
                                    else op = opGtrEq;
                                    break;
                                case 0x40:
                                    if (condOp == opEquals) op = opEquals;
                                    else op = opNotEqual;
                                    break;
                                default:
                                    std::cerr << "BranchStatement::simplify: "
                                        "k is " << std::hex << k << "\n";
                                    assert(0);
                            }
                            break;
                        default:
                            std::cerr << "BranchStatement::simplify: Mask is "
                                << std::hex << mask << "\n";
                            assert(0);
                    }
                }
                if (op != opWild) {
                    pCond = new Binary(op,
                        left1->getSubExp2()->getSubExp1(),
                        left1->getSubExp2()->getSubExp2()->getSubExp1());
                    return true;      // This is now a float comparison
                }
            }
        }
    }
    return false;
}


void BranchStatement::simplify() {
    if (pCond) {
        if (condToRelational(pCond, jtCond))
            bFloat = true;
    }
}

void BranchStatement::subscriptVar(Exp* e, Statement* def) {
    if (pCond)
        pCond = pCond->expSubscriptVar(e, def);
}

/**********************************
 * CaseStatement methods
 **********************************/
/*==============================================================================
 * FUNCTION:        CaseStatement::CaseStatement
 * OVERVIEW:        Constructor.
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
CaseStatement::CaseStatement() :
    pSwitchInfo(NULL) {
    kind = STMT_CASE;
}

/*==============================================================================
 * FUNCTION:        CaseStatement::~CaseStatement
 * OVERVIEW:        Destructor
 * NOTE:            Don't delete the pSwitchVar; it's always a copy of something
 *                  else (so don't delete twice)
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
CaseStatement::~CaseStatement() {
    if (pSwitchInfo)
        ;//delete pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        CaseStatement::getSwitchInfo
 * OVERVIEW:        Return a pointer to a struct with switch information in it
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
SWITCH_INFO* CaseStatement::getSwitchInfo() {
    return pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        CaseStatement::setSwitchInfo
 * OVERVIEW:        Set a pointer to a SWITCH_INFO struct
 * PARAMETERS:      Pointer to SWITCH_INFO struct
 * RETURNS:         <nothing>
 *============================================================================*/
void CaseStatement::setSwitchInfo(SWITCH_INFO* psi) {
    pSwitchInfo = psi;
}

/*==============================================================================
 * FUNCTION:        CaseStatement::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         True if any change
 *============================================================================*/
bool CaseStatement::searchAndReplace(Exp* search, Exp* replace) {
    GotoStatement::searchAndReplace(search, replace);
    bool ch = false;
    if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar =
          pSwitchInfo->pSwitchVar->searchReplaceAll(search, replace, ch);
    return ch;
}

/*==============================================================================
 * FUNCTION:        CaseStatement::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool CaseStatement::searchAll(Exp* search, std::list<Exp*> &result) {
    return GotoStatement::searchAll(search, result) ||
        ( pSwitchInfo && pSwitchInfo->pSwitchVar &&
          pSwitchInfo->pSwitchVar->searchAll(search, result) );
}

/*==============================================================================
 * FUNCTION:        CaseStatement::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 *                  indent: number of columns to skip
 * RETURNS:         Nothing
 *============================================================================*/
void CaseStatement::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::setw(4) << std::dec << number << " ";
    if (pSwitchInfo == NULL) {
        os << "CASE [";
        if (pDest == NULL)
            os << "*no dest*";
        else os << pDest;
        os << "] ";
    } else
        os << "SWITCH(" << pSwitchInfo->pSwitchVar << ")\n";
}


/*==============================================================================
 * FUNCTION:        CaseStatement::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new Statement that is a clone of this one
 *============================================================================*/
Statement* CaseStatement::clone() {
    CaseStatement* ret = new CaseStatement();
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    ret->pSwitchInfo = new SWITCH_INFO;
    *ret->pSwitchInfo = *pSwitchInfo;
    ret->pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->clone();
    // Statement members
    ret->pbb = pbb;
    ret->proc = proc;
    ret->number = number;
    return ret;
}

// visit this rtl
bool CaseStatement::accept(StmtVisitor* visitor) {
    return visitor->visit(this);
}

void CaseStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    // dont generate any code for switches, they will be handled by the bb
}

bool CaseStatement::usesExp(Exp *e) {
    // Before a switch statement is recognised, pDest is non null
    if (pDest)
        return *pDest == *e;
    // After a switch statement is recognised, pDest is null, and pSwitchInfo->
    // pSwitchVar takes over
    if (pSwitchInfo->pSwitchVar)
        return *pSwitchInfo->pSwitchVar == *e;
    return false;
}

void CaseStatement::subscriptVar(Exp* e, Statement* def) {
    if (pDest)
        pDest = pDest->expSubscriptVar(e, def);
    else if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->expSubscriptVar(
          e, def);
}

bool CaseStatement::doReplaceRef(Exp* from, Exp* to) {
    bool change;
    if (pDest) {
        pDest = pDest->searchReplaceAll(from, to, change);
        pDest = pDest->simplify();
        return false;
    }
    assert(pSwitchInfo && pSwitchInfo->pSwitchVar);
    pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->searchReplaceAll(
      from, to, change);
    pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->simplify();
    return false;
}

// Convert from SSA form
void CaseStatement::fromSSAform(igraph& ig) {
    if (pDest) {
        pDest = pDest->fromSSA(ig);
        return;
    }
    if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->fromSSA(ig); 
}

void CaseStatement::simplify() {
    if (pDest)
        pDest = pDest->simplify();
    else if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->simplify();
}

/**********************************
 *      CallStatement methods
 **********************************/

/*============================================================================== * FUNCTION:         CallStatement::CallStatement
 * OVERVIEW:         Constructor for a call that we have extra information
 *                   for and is part of a prologue.
 * PARAMETERS:       returnTypeSize - the size of a return union, struct or quad
 *                     floating point value
 * RETURNS:          <nothing>
 *============================================================================*/
CallStatement::CallStatement(int returnTypeSize /*= 0*/):  
      returnTypeSize(returnTypeSize), returnAfterCall(false) {
    kind = STMT_CALL;
    procDest = NULL;
}

/*==============================================================================
 * FUNCTION:      CallStatement::~CallStatement
 * OVERVIEW:      Destructor
 * PARAMETERS:    BB - the enclosing basic block of this call
 * RETURNS:       <nothing>
 *============================================================================*/
CallStatement::~CallStatement() {
	unsigned int i;
    for (i = 0; i < arguments.size(); i++)
        ;//delete arguments[i];
    for (i = 0; i < returns.size(); i++)
        ;//delete returns[i];
}

/*==============================================================================
 * FUNCTION:      CallStatement::getArguments
 * OVERVIEW:      Return a copy of the locations that have been determined
 *                as the actual arguments for this call.
 * PARAMETERS:    <none>
 * RETURNS:       A reference to the list of arguments
 *============================================================================*/
std::vector<Exp*>& CallStatement::getArguments() {
    return arguments;
}

int CallStatement::getNumReturns() {
    return returns.size();
}

Exp *CallStatement::getReturnExp(int i) {
    return returns[i];
}

int CallStatement::findReturn(Exp *e) {
    for (unsigned i = 0; i < returns.size(); i++)
        if (*returns[i] == *e)
            return i;
    return -1;
}

void CallStatement::removeReturn(Exp *e)
{
    int i = findReturn(e);
    if (i != -1) {
        for (unsigned j = i+1; j < returns.size(); j++)
            returns[j-1] = returns[j];
        returns.resize(returns.size()-1);
    }
}

void CallStatement::addReturn(Exp *e)
{
    returns.push_back(e);
}

Exp *CallStatement::getProven(Exp *e) {
    if (procDest)
        return procDest->getProven(e);
    return NULL;
}

Exp *CallStatement::substituteParams(Exp *e)
{
    e = e->clone();
    LocationSet locs;
    e->addUsedLocs(locs);
    LocationSet::iterator xx;
    for (xx = locs.begin(); xx != locs.end(); xx++) {
        Exp *r = findArgument(*xx);
        if (r == NULL) continue;
        bool change;
        e = e->searchReplaceAll(*xx, r, change);
    }
    return e->simplifyArith()->simplify();
}

Exp *CallStatement::findArgument(Exp *e) {
    int n = -1;
    if (!m_isComputed && procDest) {
        n = procDest->getSignature()->findParam(e);
        if (n != -1)
            return arguments[n];
        n = procDest->getSignature()->findImplicitParam(e);
        if (n != -1)
            return implicitArguments[n];
    } else {
        std::vector<Exp*> &params = proc->getProg()->getDefaultParams();
        if (params.size() != implicitArguments.size()) {
            LOG << "eep. " << implicitArguments.size() << " args ";
            if (procDest) {
                LOG << procDest->getName() << " ";
                LOG << "(" << procDest->getSignature()->getNumParams()
                          << " params) ";
            } else
                LOG << "(no dest) ";
            for (int i = 0; i < (int)implicitArguments.size(); i++)
                LOG << implicitArguments[i] << " ";
            LOG << "\n";
        }
        assert(params.size() == implicitArguments.size());
        for (unsigned i = 0; i < params.size(); i++)
            if (*params[i] == *e) {
                n = i;
                break;
            }
        if (n != -1)
            return implicitArguments[n];
    }
    assert(n == -1);
    return NULL;
}

void CallStatement::addArgument(Exp *e)
{
    e = substituteParams(e);
    arguments.push_back(e);
    // These expressions can end up becoming locals
    Location *l = dynamic_cast<Location*>(e);
    if (l)
        l->setProc(proc);
}

Type *CallStatement::getArgumentType(int i) {
    assert(i < (int)arguments.size());
    if (procDest)
        return procDest->getSignature()->getParamType(i);
    return NULL;
}

/*==============================================================================
 * FUNCTION:      CallStatement::setArguments
 * OVERVIEW:      Set the arguments of this call.
 * PARAMETERS:    arguments - the list of locations that reach this call
 * RETURNS:       <nothing>
 *============================================================================*/
void CallStatement::setArguments(std::vector<Exp*>& arguments) {
    this->arguments = arguments;
    std::vector<Exp*>::iterator ll;
    for (ll = arguments.begin(); ll != arguments.end(); ll++) {
        Location *l = dynamic_cast<Location*>(*ll);
        if (l) {
            l->setProc(proc);
        }
    }
}

void CallStatement::setImpArguments(std::vector<Exp*>& arguments) {
    this->implicitArguments = arguments;
    std::vector<Exp*>::iterator ll;
    for (ll = implicitArguments.begin(); ll != implicitArguments.end(); ll++) {
        Location *l = dynamic_cast<Location*>(*ll);
        if (l) {
            l->setProc(proc);
        }
    }
}

/*==============================================================================
 * FUNCTION:      CallStatement::setSigArguments
 * OVERVIEW:      Set the arguments of this call based on signature info
 * PARAMETERS:    None
 * RETURNS:       <nothing>
 *============================================================================*/
void CallStatement::setSigArguments() {
    Signature *sig;
    if (procDest == NULL) {
        if (proc == NULL) return; // do it later
        // computed calls must have their arguments initialized to something 
        std::vector<Exp*> &params = proc->getProg()->getDefaultParams();
        implicitArguments.resize(params.size());
		unsigned i;
        for (i = 0; i < params.size(); i++) {
            implicitArguments[i] = params[i]->clone();
            implicitArguments[i]->fixLocationProc(proc);
        }
        std::vector<Exp*> &rets = proc->getProg()->getDefaultReturns();
        returns.resize(0);
        for (i = 0; i < rets.size(); i++)
            if (!(*rets[i] == *pDest))
                returns.push_back(rets[i]->clone());
        return;
    } else 
        sig = procDest->getSignature();
    
    int n = sig->getNumParams();
    arguments.resize(n);
	int i;
    for (i = 0; i < n; i++) {
        Exp *e = sig->getArgumentExp(i);
        assert(e);
        arguments[i] = e->clone();
        Location *l = dynamic_cast<Location*>(e);
        if (l) {
            l->setProc(proc);
        }
    }
    if (sig->hasEllipsis()) {
        // Just guess 4 parameters for now
        for (int i = 0; i < 4; i++)
            arguments.push_back(sig->getArgumentExp(
                                    arguments.size())->clone());
    }
    if (procDest)
        procDest->addCaller(this);

    n = sig->getNumImplicitParams();
    implicitArguments.resize(n);
    for (i = 0; i < n; i++) {
        Exp *e = sig->getImplicitParamExp(i);
        assert(e);
        implicitArguments[i] = e->clone();
        implicitArguments[i]->fixLocationProc(proc);
    }
 
    // initialize returns
    returns.clear();
    for (i = 0; i < sig->getNumReturns(); i++)
        returns.push_back(sig->getReturnExp(i)->clone());

    if (procDest == NULL)
        ;//delete sig;
}

#if 0
/*==============================================================================
 * FUNCTION:      CallStatement::getReturnLoc
 * OVERVIEW:      Return the location that will be used to hold the value
 *                  returned by this call.
 * PARAMETERS:    <none>
 * RETURNS:       ptr to the location that will be used to hold the return value
 *============================================================================*/
Exp* CallStatement::getReturnLoc() {
    if (returns.size() == 1)
        return returns[0];
    return NULL;
}

Type* CallStatement::getLeftType() {
    if (procDest == NULL)
        return new VoidType();
    return procDest->getSignature()->getReturnType();
}
#endif

/*==============================================================================
 * FUNCTION:         CallStatement::returnsStruct
 * OVERVIEW:         Returns true if the function called by this call site
 *                   returns an aggregate value (i.e a struct, union or quad
 *                   floating point value).
 * PARAMETERS:       <none>
 * RETURNS:          the called function returns an aggregate value
 *============================================================================*/
bool CallStatement::returnsStruct() {
    return (returnTypeSize != 0);
}

bool CallStatement::search(Exp* search, Exp*& result) {
    result = NULL;
	unsigned int i;
    for (i = 0; i < returns.size(); i++) {
        if (*returns[i] == *search) {
            result = returns[i];
            return true;
        }
        if (returns[i]->search(search, result)) return true;
    }
    for (i = 0; i < arguments.size(); i++) {
        if (*arguments[i] == *search) {
            result = arguments[i];
            return true;
        }
        if (arguments[i]->search(search, result)) return true;
    }
    for (i = 0; i < implicitArguments.size(); i++) {
        if (*implicitArguments[i] == *search) {
            result = implicitArguments[i];
            return true;
        }
        if (implicitArguments[i]->search(search, result)) return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        CallStatement::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         True if any change
 *============================================================================*/
bool CallStatement::searchAndReplace(Exp* search, Exp* replace) {
    bool change = GotoStatement::searchAndReplace(search, replace);
	unsigned int i;
    for (i = 0; i < returns.size(); i++) {
        bool ch;
        returns[i] = returns[i]->searchReplaceAll(search, replace, ch);
        change |= ch;
    }
    for (i = 0; i < arguments.size(); i++) {
        bool ch;
        arguments[i] = arguments[i]->searchReplaceAll(search, replace, ch);
        change |= ch;
    }
    for (i = 0; i < implicitArguments.size(); i++) {
        bool ch;
        implicitArguments[i] = implicitArguments[i]->searchReplaceAll(
            search, replace, ch);
        change |= ch;
    }
    return change;
}

/*==============================================================================
 * FUNCTION:        CallStatement::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool CallStatement::searchAll(Exp* search, std::list<Exp *>& result) {
    bool found = false;
	unsigned int i;
    for (i = 0; i < arguments.size(); i++)
        if (arguments[i]->searchAll(search, result))
            found = true;
    for (i = 0; i < implicitArguments.size(); i++)
        if (implicitArguments[i]->searchAll(search, result))
            found = true;
     for (i = 0; i < returns.size(); i++)
        if (returns[i]->searchAll(search, result))
            found = true;
    return found;
}

/*==============================================================================
 * FUNCTION:        CallStatement::print
 * OVERVIEW:        Write a text representation of this RTL to the given stream
 * PARAMETERS:      os: stream to write to
 * RETURNS:         Nothing
 *============================================================================*/
void CallStatement::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::setw(4) << std::dec << number << " ";
 
    os << "CALL ";
    if (procDest)
        os << procDest->getName();
    else if (pDest == NULL)
            os << "*no dest*";
    else {
        if (pDest->isIntConst())
            os << "0x" << std::hex << ((Const*)pDest)->getInt();
        else
            pDest->print(os, withDF);       // Could still be an expression
    }

    // Print the actual arguments of the call
    os << "(";
	unsigned int i;
    for (i = 0; i < arguments.size(); i++) {
        if (i != 0)
            os << ", ";
        os << arguments[i];
    }
    os << " implicit: ";
    for (i = 0; i < implicitArguments.size(); i++) {
        if (i != 0)
            os << ", ";
        os << implicitArguments[i];
    }
    os << ")";

    // And the return locations 
    if (getNumReturns()) {
        os << " { ";
        for (int i = 0; i < getNumReturns(); i++) {
            if (i != 0)
                os << ", ";
            os << getReturnExp(i);
        }
        os << " }";
    }
}

/*==============================================================================
 * FUNCTION:         CallStatement::setReturnAfterCall
 * OVERVIEW:         Sets a bit that says that this call is effectively followed
 *                      by a return. This happens e.g. on Sparc when there is a
 *                      restore in the delay slot of the call
 * PARAMETERS:       b: true if this is to be set; false to clear the bit
 * RETURNS:          <nothing>
 *============================================================================*/
void CallStatement::setReturnAfterCall(bool b) {
    returnAfterCall = b;
}

/*==============================================================================
 * FUNCTION:         CallStatement::isReturnAfterCall
 * OVERVIEW:         Tests a bit that says that this call is effectively
 *                      followed by a return. This happens e.g. on Sparc when
 *                      there is a restore in the delay slot of the call
 * PARAMETERS:       none
 * RETURNS:          True if this call is effectively followed by a return
 *============================================================================*/
bool CallStatement::isReturnAfterCall() {
    return returnAfterCall;
}

/*==============================================================================
 * FUNCTION:        CallStatement::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new Statement, a clone of this CallStatement
 *============================================================================*/
Statement* CallStatement::clone() {
    CallStatement* ret = new CallStatement();
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    int n = arguments.size();
	int i;
    for (i=0; i < n; i++)
        ret->arguments.push_back(arguments[i]->clone());
    n = implicitArguments.size();
    for (i=0; i < n; i++)
        ret->implicitArguments.push_back(implicitArguments[i]->clone());
    // Statement members
    ret->pbb = pbb;
    ret->proc = proc;
    ret->number = number;
    return ret;
}

// visit this rtl
bool CallStatement::accept(StmtVisitor* visitor) {
    return visitor->visit(this);
}

Proc* CallStatement::getDestProc() {
    return procDest; 
}

void CallStatement::setDestProc(Proc* dest) { 
    assert(dest);
    assert(procDest == NULL);
    procDest = dest;
}

void CallStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    LocationSet defs;
    getDefinitions(defs);

    Proc *p = getDestProc();

    if (p == NULL && isComputed()) {
        hll->AddIndCallStatement(indLevel, pDest, arguments);
        return;
    }

#if 0
    LOG << "call: " << this;
    LOG << " in proc " << proc->getName() << "\n";
#endif
    assert(p);
    if (p->isLib() && *p->getSignature()->getPreferedName()) {
        std::vector<Exp*> args;
        for (unsigned int i = 0; i < p->getSignature()->getNumPreferedParams();
             i++)
            args.push_back(arguments[p->getSignature()->getPreferedParam(i)]);
        hll->AddCallStatement(indLevel, p,  
                              p->getSignature()->getPreferedName(),
                              args, defs);
    } else
        hll->AddCallStatement(indLevel, p, p->getName(), arguments, defs);
}

void CallStatement::simplify() {
    GotoStatement::simplify();
	unsigned int i;
    for (i = 0; i < arguments.size(); i++) {
        arguments[i] = arguments[i]->simplifyArith()->simplify();
    }
    for (i = 0; i < implicitArguments.size(); i++) {
        implicitArguments[i] = implicitArguments[i]->simplifyArith()->simplify();
    }
    for (i = 0; i < returns.size(); i++) {
        returns[i] = returns[i]->simplifyArith()->simplify();
    }
}

#if 0
void CallStatement::decompile() {
    if (procDest) { 
        UserProc *p = dynamic_cast<UserProc*>(procDest);
        if (p != NULL)
            p->decompile();

        // FIXME: Likely there is a much better place to do this
        // init return location
        setIgnoreReturnLoc(false);
    } else {
        // TODO: indirect call
    }
}
#endif

// update type for expression
Type *CallStatement::updateType(Exp *e, Type *curType) {
    return curType;
}

bool CallStatement::usesExp(Exp *e) {
    Exp *where;
	unsigned int i;
    for (i = 0; i < arguments.size(); i++) {
        if (arguments[i]->search(e, where)) {
            return true;
        }
    }
    for (i = 0; i < implicitArguments.size(); i++) {
        if (implicitArguments[i]->search(e, where)) {
            return true;
        }
    }
    for (i = 0; i < returns.size(); i++) {
        if (returns[i]->isMemOf() && 
                returns[i]->getSubExp1()->search(e, where))
            return true;
    }
    if (procDest == NULL)
        // No destination (e.g. indirect call)
        // For now, just return true (overstating uses is safe)
        return true;
#if 0   // Huh? This was wrong anyway!
    if (!procDest->isLib()) {
        // Get the info that was summarised on the way down
        if (liveEntry.find(e)) return true;
    }
#endif
    return false;
}

bool CallStatement::isDefinition() 
{
    LocationSet defs;
    getDefinitions(defs);
    return defs.size() != 0;
}

void CallStatement::getDefinitions(LocationSet &defs) {
    for (int i = 0; i < getNumReturns(); i++) {
        defs.insert(getReturnExp(i));
    }
}

void CallStatement::subscriptVar(Exp* e, Statement* def) {
#if 0
    LOG << "callstatement subscriptvar " << e << " to "; 
    if (def == NULL)
        LOG << "0\n";
    else
        LOG << def->getNumber() << "\n";
#endif
    if (procDest == NULL && pDest)
        pDest = pDest->expSubscriptVar(e, def);
	unsigned int i;
    for (i = 0; i < returns.size(); i++) 
        if (returns[i]->getOper() == opMemOf) {
            returns[i]->refSubExp1() = 
                returns[i]->getSubExp1()->expSubscriptVar(e, def);
        }
    for (i = 0; i < arguments.size(); i++) {
        arguments[i] = arguments[i]->expSubscriptVar(e, def);
    }
    for (i = 0; i < implicitArguments.size(); i++) {
        implicitArguments[i] = implicitArguments[i]->expSubscriptVar(e, def);
    }
}

bool CallStatement::doReplaceRef(Exp* from, Exp* to) {
    bool change = false;
    bool convertIndirect = false;
    if (procDest == NULL && pDest) {
        pDest = pDest->searchReplaceAll(from, to, change);
        if (change) {
            if (VERBOSE)
                LOG << "propagated into call dest " << pDest << "\n";
            Exp *e = pDest;
            if (pDest->isSubscript())
                e = ((RefExp*)e)->getSubExp1();
            if (e->getOper() == opArraySubscript && 
                  ((Binary*)e)->getSubExp2()->isIntConst() &&
                  ((Const*)(((Binary*)e)->getSubExp2()))->getInt() == 0)
                e = ((Binary*)e)->getSubExp1();
            // Can actually have name{0}[0]{0} !!
            if (e->isSubscript())
                e = ((RefExp*)e)->getSubExp1();
            if (e->getOper() == opGlobal) {
                char *nam = ((Const*)e->getSubExp1())->getStr();
                Proc *p = proc->getProg()->findProc(nam);
                if (p == NULL)
                    p = proc->getProg()->getLibraryProc(nam);
                if (VERBOSE)
                    LOG << "this is a global '" << nam << "'\n";
                if (p) {
                    if (VERBOSE)
                        LOG << "this is a proc " << p->getName() << "\n";
                    // we need to:
                    // 1) replace the current return set with the return set
                    //    of the new procDest
                    // 2) call fixCallRefs on the enclosing procedure
                    // 3) fix the arguments (this will only affect the implicit 
                    //    arguments, the regular arguments should be empty at
                    //    this point)
                    // 3a replace current arguments with those of the new proc
                    // 4) change this to a non-indirect call
                    procDest = p;
                    Signature *sig = p->getSignature();
                    // 1
                    //LOG << "1\n";
                    returns.resize(sig->getNumReturns());
                    int i;
                    for (i = 0; i < sig->getNumReturns(); i++)
                        returns[i] = sig->getReturnExp(i)->clone();
                    // 2
                    //LOG << "2\n";
                    proc->fixCallRefs();
                    // 3
                    //LOG << "3\n";
                    std::vector<Exp*> &params = proc->getProg()
                      ->getDefaultParams();
                    std::vector<Exp*> oldargs = implicitArguments;
                    std::vector<Exp*> newimpargs;
                    newimpargs.resize(sig->getNumImplicitParams());
                    for (i = 0; i < sig->getNumImplicitParams(); i++) {
                        bool gotsub = false;
                        for (unsigned j = 0; j < params.size(); j++)
                            if (*params[j] == *sig->getImplicitParamExp(i)) {
                                newimpargs[i] = oldargs[j];
                                gotsub = true;
                                break;
                            }
                        if (!gotsub) {
                            newimpargs[i] =
                              sig->getImplicitParamExp(i)->clone();
                            if (newimpargs[i]->getOper() == opMemOf) {
                                newimpargs[i]->refSubExp1() = 
                                    substituteParams(newimpargs[i]->
                                    getSubExp1());
                            }
                        }
                    }
                    // 3a Do the same with the regular arguments
                    assert(arguments.size() == 0);
                    std::vector<Exp*> newargs;
                    newargs.resize(sig->getNumParams());
                    for (i = 0; i < sig->getNumParams(); i++) {
                        bool gotsub = false;
                        for (unsigned j = 0; j < params.size(); j++)
                            if (*params[j] == *sig->getParamExp(i)) {
                                newargs[i] = oldargs[j];
                                // Got something to substitute
                                gotsub = true;
                                break;
                            }
                        if (!gotsub) {
                            Exp* parami = sig->getParamExp(i);
                            newargs[i] = parami->clone();
                            if (newargs[i]->getOper() == opMemOf) {
                                newargs[i]->refSubExp1() = 
                                    substituteParams(newargs[i]->getSubExp1());
                            }
                        }
                    }
                    // change em
                    arguments = newargs;
                    assert((int)arguments.size() == sig->getNumParams());
                    implicitArguments = newimpargs;
                    assert((int)implicitArguments.size() ==
                      sig->getNumImplicitParams());
                    // 4
                    //LOG << "4\n";
                    m_isComputed = false;
                    proc->undoComputedBB(this);
                    proc->addCallee(procDest);
                    procDest->printDetailsXML();
                    convertIndirect = true;
                }
            }
        }
    }
	unsigned int i;
    for (i = 0; i < returns.size(); i++)
        if (returns[i]->getOper() == opMemOf) {
            Exp *e = findArgument(returns[i]->getSubExp1());
            if (e)
                returns[i]->refSubExp1() = e->clone();
            returns[i]->refSubExp1() = 
                returns[i]->getSubExp1()->searchReplaceAll(from, to, change);
            // Simplify is very expensive, especially if it happens to
            // reference a phi statement (attempts proofs)
            if (change) {
                returns[i] = returns[i]->simplifyArith()->simplify();
                if (0 & VERBOSE)
                    LOG << "doReplaceRef: updated return[" << i << "] with " <<
                      returns[i] << "\n";
            }
        }    
    for (i = 0; i < arguments.size(); i++) {
        arguments[i] = arguments[i]->searchReplaceAll(from, to, change);
        if (change) {
            arguments[i] = arguments[i]->simplifyArith()->simplify();
            if (0 & VERBOSE)
                LOG << "doReplaceRef: updated argument[" << i << "] with " <<
                  arguments[i] << "\n";
        }
    }
    for (i = 0; i < implicitArguments.size(); i++) {
        // Don't replace the implicit argument if it matches whole expression.
        // A large part of the use of these is to allow fixCallRefs to change
        // a definition of a location (say sp) by what the function does with
        // it (maybe replaces it with sp+4). If you substitute sp{K} with say
        // sp{K-3}+4, then it won't do its job with fixCallRefs.
        if (!(*implicitArguments[i] == *from)) {
            implicitArguments[i] = implicitArguments[i]->
              searchReplaceAll(from, to, change);
            if (change) {
                implicitArguments[i] =
                  implicitArguments[i]->simplifyArith()->simplify();
                if (0 & VERBOSE)
                    LOG << "doReplaceRef: updated implicitArguments[" << i <<
                      "] with " << implicitArguments[i] << "\n";
            }
        }
    }
    return convertIndirect;
}

Exp* CallStatement::getArgumentExp(int i)
{
    assert(i < (int)arguments.size());
    return arguments[i];
}

Exp* CallStatement::getImplicitArgumentExp(int i)
{
    assert(i < (int)implicitArguments.size());
    return implicitArguments[i];
}

void CallStatement::setArgumentExp(int i, Exp *e)
{
    assert(i < (int)arguments.size());
    arguments[i] = e->clone();
}

int CallStatement::getNumArguments()
{
    return arguments.size();
}

void CallStatement::setNumArguments(int n) {
    int oldSize = arguments.size();
    arguments.resize(n);
    // printf, scanf start with just 2 arguments
    for (int i = oldSize; i < n; i++) {
        arguments[i] = procDest->getSignature()->getArgumentExp(i)->clone();
    }
}

void CallStatement::removeArgument(int i)
{
    for (unsigned j = i+1; j < arguments.size(); j++)
        arguments[j-1] = arguments[j];
    arguments.resize(arguments.size()-1);
}

void CallStatement::removeImplicitArgument(int i)
{
    for (unsigned j = i+1; j < implicitArguments.size(); j++)
        implicitArguments[j-1] = implicitArguments[j];
    implicitArguments.resize(implicitArguments.size()-1);
}

// Convert from SSA form
void CallStatement::fromSSAform(igraph& ig) {
    if (pDest)
        pDest = pDest->fromSSA(ig);
    int n = arguments.size();
	int i;
    for (i=0; i < n; i++) {
        arguments[i] = arguments[i]->fromSSA(ig);
    }
    n = implicitArguments.size();
    for (i=0; i < n; i++) {
        implicitArguments[i] = implicitArguments[i]->fromSSA(ig);
    }
    n = returns.size();
    for (i=0; i < n; i++) {
        returns[i] = returns[i]->fromSSAleft(ig, this);
    }
}


// Insert actual arguments to match the formal parameters
// This is called very late, after all propagation
void CallStatement::insertArguments(StatementSet& rs) {
    // FIXME: Fix this, or delete the whole function
#if 0
    if (procDest == NULL) return;
    if (procDest->isLib()) return;
    Signature* sig = procDest->getSignature();
    int num = sig->getNumParams();
    //arguments.resize(num);
    // Get the set of definitions that reach this call
    StatementSet rd;
    getBB()->getReachInAt(this, rd, 2);
    StatementSet empty;
    for (int i=0; i<num; i++) {
        Exp* loc = sig->getArgumentExp(i)->clone();
        // Needs to be subscripted with everything that reaches the parameters
        // FIXME: need to be sensible about memory depths
        loc->updateRefs(rd, 0, rs);
        propagateTo(0, empty);
        loc->updateRefs(rd, 1, rs);
        propagateTo(1, empty);
        arguments.push_back(loc);
    }
#endif
}

Exp *Statement::processConstant(Exp *e, Type *t, Prog *prog)
{
    if (t == NULL) return e;
    NamedType *nt = NULL;
    if (t->isNamed()) {
        nt = (NamedType*)t;
        t = ((NamedType*)t)->resolvesTo();
    }
    if (t == NULL) return e;
    // char* and a constant
    if (e->isIntConst()) {
        if (nt && nt->getName() == "LPCWSTR") {
            ADDRESS u = ((Const*)e)->getAddr();
            // TODO
            LOG << "possible wide char string at " << u << "\n";
        }
        if (t->resolvesToPointer()) {
            PointerType *pt = t->asPointer();
            Type *points_to = pt->getPointsTo();
            if (points_to->resolvesToChar()) {
                ADDRESS u = ((Const*)e)->getAddr();
                char *str = 
                    prog->getStringConstant(u, true);
                if (str) {
                    e = new Const(escapeStr(str));
                    // Check if we may have guessed this global incorrectly
                    // (usually as an array of char)
                    Prog* prog = proc->getProg();
                    const char* nam = prog->getGlobalName(u);
                    if (nam) prog->setGlobalType(nam,
                        new PointerType(new CharType()));
                } else {
                    proc->getProg()->globalUsed(u);
                    const char *nam = proc->getProg()->getGlobalName(u);
                    if (nam)
                        e = Location::global(nam, proc);
                }
            }
            if (points_to->resolvesToFunc()) {
                ADDRESS a = ((Const*)e)->getAddr();
                if (VERBOSE)
                    LOG << "found function pointer with constant value "
                        << "of type " << pt->getCtype() 
                        << ".  Decoding address " << a << "\n";
                if (!Boomerang::get()->noDecodeChildren)
                    prog->decodeExtraEntrypoint(a);
                Proc *p = prog->findProc(a);
                if (p) {
                    Signature *sig = points_to->asFunc()->getSignature()->clone();
                    if (sig->getName() == NULL ||
                        strlen(sig->getName()) == 0 || 
                        !strcmp(sig->getName(), "<ANON>") ||
                        prog->findProc(sig->getName()) != NULL)
                        sig->setName(p->getName());
                    else
                        p->setName(sig->getName());
                    p->setSignature(sig);
                    e = Location::global(p->getName(), proc);
                }
            }
        } else if (t->resolvesToFloat()) {
            e = new Ternary(opItof, new Const(32), new Const(t->getSize()), e);
        } 
    }
#if 0
    if (t->isPointer() && e->getOper() != opAddrOf) {
        e = new Unary(opAddrOf, Location::memOf(e));
    }
#endif
    
    return e;
}


Type *Statement::getTypeFor(Exp *e, Prog *prog)
{
    Type *ty = NULL;
    if (e->getOper() == opGlobal) {
        const char *nam = ((Const*)e->getSubExp1())->getStr();
        ty = prog->getGlobalType((char*)nam);
    }
    if (e->getOper() == opLocal) {
        const char *nam = ((Const*)e->getSubExp1())->getStr();
        ty = proc->getLocalType((char*)nam);
    }
    if (e->getOper() == opMemberAccess) {
        Type *tsubexp1 = getTypeFor(e->getSubExp1(), prog);
        if (tsubexp1->resolvesToCompound()) {
            CompoundType *compound = tsubexp1->asCompound();
            const char *nam = ((Const*)e->getSubExp2())->getStr();
            ty = compound->getType((char*)nam);
        }
    }
    if (e->getOper() == opSubscript) {
        ty = getTypeFor(e->getSubExp1(), prog);
    }
    // MVE: not sure if this is right
    if (ty == NULL) ty = e->getType();  // May be m[blah].member
    return ty;
}

void CallStatement::processConstants(Prog *prog) {
    for (unsigned i = 0; i < arguments.size(); i++) {
        Type *t = getArgumentType(i);
        Exp *e = arguments[i];
    
        arguments[i] = processConstant(e, t, prog);
    }

    // hack
    if (getDestProc() && getDestProc()->isLib()) {
        int sp = proc->getSignature()->getStackRegister(prog);
        removeReturn(Location::regOf(sp));
		unsigned int i;
        for (i = 0; i < implicitArguments.size(); i++)
            if (*getDestProc()->getSignature()->getImplicitParamExp(i) == *Location::regOf(sp)) {
                implicitArguments[i] = new Const(0);
                break;
            }
        for (i = 0; i < implicitArguments.size(); i++)
            if (*getDestProc()->getSignature()->getImplicitParamExp(i) ==
                  *Location::memOf(Location::regOf(sp))) {
                implicitArguments[i] = new Const(0);
                break;
            }
    }

    // This code was in CallStatement::doReplaceRef()
    if (getDestProc() && getDestProc()->getSignature()->hasEllipsis()) {
        // functions like printf almost always have too many args
        std::string name(getDestProc()->getName());
        if ((name == "printf" || name == "scanf") &&
              getArgumentExp(0)->isStrConst()) {
            char *str = ((Const*)getArgumentExp(0))->getStr();
            // actually have to parse it
            int n = 1;      // Number of %s plus 1 = number of args
            char *p = str;
            while ((p = strchr(p, '%'))) {
                // special hack for scanf
                if (name == "scanf") {
                    setArgumentExp(n, new Unary(opAddrOf,
                        Location::memOf(getArgumentExp(n), proc)));
                }
                p++;
                switch(*p) {
                    case '%':
                        break;
                        // TODO: there's type information here
                    default:
                        n++;
                }
                p++;
            }
            setNumArguments(n);
        }
    }

}

/**********************************
 * ReturnStatement methods
 **********************************/

/*==============================================================================
 * FUNCTION:         ReturnStatement::ReturnStatement
 * OVERVIEW:         Constructor.
 * PARAMETERS:       None
 * RETURNS:          <nothing>
 *============================================================================*/
ReturnStatement::ReturnStatement() : nBytesPopped(0), retAddr(NO_ADDRESS) {
    kind = STMT_RET;
}

/*==============================================================================
 * FUNCTION:         ReturnStatement::~ReturnStatement
 * OVERVIEW:         Destructor.
 * PARAMETERS:       <none>
 * RETURNS:          <nothing>
 *============================================================================*/
ReturnStatement::~ReturnStatement() {
}

/*==============================================================================
 * FUNCTION:        ReturnStatement::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new Statement, a clone of this ReturnStatement
 *============================================================================*/
Statement* ReturnStatement::clone() {
    ReturnStatement* ret = new ReturnStatement();
    ret->pDest = NULL;                      // pDest should be null
    ret->m_isComputed = m_isComputed;
    // Statement members
    ret->pbb = pbb;
    ret->proc = proc;
    ret->number = number;
    return ret;
}

// visit this rtl
bool ReturnStatement::accept(StmtVisitor* visitor) {
    return visitor->visit(this);
}

void ReturnStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) 
{
    hll->AddReturnStatement(indLevel, returns);
}

void ReturnStatement::simplify() {
    for (unsigned i = 0; i < returns.size(); i++)
        returns[i] = returns[i]->simplify();
}

void ReturnStatement::setSigArguments() {
    for (int i = 0; i < proc->getSignature()->getNumReturns(); i++)
        returns.push_back(proc->getSignature()->getReturnExp(i)->clone());
}

void ReturnStatement::removeReturn(int n)
{
    int i = n;
    if (i != -1) {
        for (unsigned j = i+1; j < returns.size(); j++)
            returns[j-1] = returns[j];
        returns.resize(returns.size()-1);
    }
}

void ReturnStatement::addReturn(Exp *e)
{
    returns.push_back(e);
}

// Convert from SSA form
void ReturnStatement::fromSSAform(igraph& ig) {
    int n = returns.size();
    for (int i=0; i < n; i++) {
        returns[i] = returns[i]->fromSSA(ig);
    }
}

void ReturnStatement::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::setw(4) << std::dec << number << " ";
    os << "RET ";
    for (unsigned i = 0; i < returns.size(); i++) {
        if (i != 0)
            os << ", ";
        os << returns[i];
    }
}

bool ReturnStatement::search(Exp* search, Exp*& result) {
    result = NULL;
    for (unsigned i = 0; i < returns.size(); i++) {
        if (*returns[i] == *search) {
            result = returns[i];
            return true;
        }
        if (returns[i]->search(search, result)) return true;
    }
    return false;
}

bool ReturnStatement::searchAndReplace(Exp* search, Exp* replace) {
    bool change = GotoStatement::searchAndReplace(search, replace);
    for (int i = 0; i < (int)returns.size(); i++) {
        bool ch;
        returns[i] = returns[i]->searchReplaceAll(search, replace, ch);
        change |= ch;
    }
    return change;
}

bool ReturnStatement::searchAll(Exp* search, std::list<Exp *>& result) {
    bool found = false;
    for (unsigned i = 0; i < returns.size(); i++)
        if (returns[i]->searchAll(search, result))
            found = true;
    return found;
}

bool ReturnStatement::usesExp(Exp *e) {
    Exp *where;
    for (unsigned i = 0; i < returns.size(); i++) {
        if (returns[i]->search(e, where)) {
            return true;
        }
    }
    return false;
}

void ReturnStatement::subscriptVar(Exp* e, Statement* def) {
    for (unsigned i = 0; i < returns.size(); i++) {
        returns[i] = returns[i]->expSubscriptVar(e, def);
    }
}

bool ReturnStatement::doReplaceRef(Exp* from, Exp* to) {
    bool change = false;
    for (unsigned i = 0; i < returns.size(); i++) {
        returns[i] = returns[i]->searchReplaceAll(from, to, change);
        returns[i] = returns[i]->simplifyArith()->simplify();
    }
    return false;
}
 
/**********************************************************************
 * BoolStatement methods
 * These are for statements that set a destination (usually to 1 or 0)
 * depending in a condition code (e.g. Pentium)
 **********************************************************************/

/*==============================================================================
 * FUNCTION:         BoolStatement::BoolStatement
 * OVERVIEW:         Constructor.
 * PARAMETERS:       sz: size of the assignment
 * RETURNS:          <N/a>
 *============================================================================*/
BoolStatement::BoolStatement(int sz): jtCond((BRANCH_TYPE)0), pCond(NULL),
  pDest(NULL), size(sz) {
    kind = STMT_BOOL;
}

/*==============================================================================
 * FUNCTION:        BoolStatement::~BoolStatement
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
BoolStatement::~BoolStatement() {
    if (pCond)
        ;//delete pCond;
}

/*==============================================================================
 * FUNCTION:        BoolStatement::setCondType
 * OVERVIEW:        Sets the BRANCH_TYPE of this jcond as well as the flag
 *                  indicating whether or not the floating point condition codes
 *                  are used.
 * PARAMETERS:      cond - the BRANCH_TYPE
 *                  usesFloat - this condional jump checks the floating point
 *                    condition codes
 * RETURNS:         a semantic string
 *============================================================================*/
void BoolStatement::setCondType(BRANCH_TYPE cond, bool usesFloat /*= false*/) {
    jtCond = cond;
    bFloat = usesFloat;
    setCondExpr(new Terminal(opFlags));
    getDest();
}

/*==============================================================================
 * FUNCTION:        BoolStatement::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * NOTE:            Not sure if this is ever going to be used
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void BoolStatement::makeSigned() {
    // Make this into a signed branch
    switch (jtCond)
    {
        case BRANCH_JUL : jtCond = BRANCH_JSL;  break;
        case BRANCH_JULE: jtCond = BRANCH_JSLE; break;
        case BRANCH_JUGE: jtCond = BRANCH_JSGE; break;
        case BRANCH_JUG : jtCond = BRANCH_JSG;  break;
        default:
            // Do nothing for other cases
            break;
    }
}

/*==============================================================================
 * FUNCTION:        BoolStatement::getCondExpr
 * OVERVIEW:        Return the Exp expression containing the HL condition.
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
Exp* BoolStatement::getCondExpr() {
    return pCond;
}

/*==============================================================================
 * FUNCTION:        BoolStatement::setCondExpr
 * OVERVIEW:        Set the Exp expression containing the HL condition.
 * PARAMETERS:      Pointer to semantic string to set
 * RETURNS:         <nothing>
 *============================================================================*/
void BoolStatement::setCondExpr(Exp* pss) {
    if (pCond) ;//delete pCond;
    pCond = pss;
}

/*==============================================================================
 * FUNCTION:        BoolStatement::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         <Nothing>
 *============================================================================*/
void BoolStatement::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::setw(4) << std::dec << number << " ";
    os << "BOOL ";
    pDest->print(os);
    os << " := CC(";
    switch (jtCond)
    {
        case BRANCH_JE:    os << "equals"; break;
        case BRANCH_JNE:   os << "not equals"; break;
        case BRANCH_JSL:   os << "signed less"; break;
        case BRANCH_JSLE:  os << "signed less or equals"; break;
        case BRANCH_JSGE:  os << "signed greater or equals"; break;
        case BRANCH_JSG:   os << "signed greater"; break;
        case BRANCH_JUL:   os << "unsigned less"; break;
        case BRANCH_JULE:  os << "unsigned less or equals"; break;
        case BRANCH_JUGE:  os << "unsigned greater or equals"; break;
        case BRANCH_JUG:   os << "unsigned greater"; break;
        case BRANCH_JMI:   os << "minus"; break;
        case BRANCH_JPOS:  os << "plus"; break;
        case BRANCH_JOF:   os << "overflow"; break;
        case BRANCH_JNOF:  os << "no overflow"; break;
        case BRANCH_JPAR:  os << "parity"; break;
    }
    os << ")";
    if (bFloat) os << ", float";
    os << std::endl;
    if (pCond)
        os << "High level: " << pCond << "\n";
}

/*==============================================================================
 * FUNCTION:        BoolStatement::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new Statement, a clone of this BoolStatement
 *============================================================================*/
Statement* BoolStatement::clone() {
    BoolStatement* ret = new BoolStatement(size);
    ret->jtCond = jtCond;
    if (pCond) ret->pCond = pCond->clone();
    else ret->pCond = NULL;
    ret->bFloat = bFloat;
    ret->size = size;
    // Statement members
    ret->pbb = pbb;
    ret->proc = proc;
    ret->number = number;
    return ret;
}

// visit this Statement
bool BoolStatement::accept(StmtVisitor* visitor) {
    return visitor->visit(this);
}

void BoolStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    assert(pDest);
    assert(pCond);
    // pDest := (pCond) ? 1 : 0
    Assign as(pDest->clone(), new Ternary(opTern, pCond->clone(),
      new Const(1), new Const(0)));
    hll->AddAssignmentStatement(indLevel, &as);
}

void BoolStatement::simplify() {
    if (pCond)
        condToRelational(pCond, jtCond);
}

void BoolStatement::getDefinitions(LocationSet &defs) 
{
    defs.insert(getLeft());
}

Type* BoolStatement::getLeftType()
{
    return new BooleanType();
}

bool BoolStatement::usesExp(Exp *e)
{
    assert(pDest && pCond);
    Exp *where = 0;
    return (pCond->search(e, where) || (pDest->isMemOf() && 
        ((Unary*)pDest)->getSubExp1()->search(e, where)));
}

void BoolStatement::processConstants(Prog *prog)
{
}

bool BoolStatement::search(Exp *search, Exp *&result)
{
    assert(pDest);
    if (pDest->search(search, result)) return true;
    assert(pCond);
    return pCond->search(search, result);
}

bool BoolStatement::searchAll(Exp* search, std::list<Exp*>& result)
{
    bool ch = false;
    assert(pDest);
    if (pDest->searchAll(search, result)) ch = true;
    assert(pCond);
    return pCond->searchAll(search, result) || ch;
}

bool BoolStatement::searchAndReplace(Exp *search, Exp *replace) {
    bool change = false;
    assert(pCond);
    assert(pDest);
    pCond = pCond->searchReplaceAll(search, replace, change);
    pDest = pDest->searchReplaceAll(search, replace, change);
    return change;
}

Type* BoolStatement::updateType(Exp *e, Type *curType) {
    ;//delete curType;
    return new BooleanType();
}

// Convert from SSA form
void BoolStatement::fromSSAform(igraph& ig) {
    pCond = pCond->fromSSA(ig); 
    pDest = pDest->fromSSAleft(ig, this);
}

bool BoolStatement::doReplaceRef(Exp* from, Exp* to) {
    searchAndReplace(from, to);
    simplify();
    return false;
}

void BoolStatement::subscriptVar(Exp* e, Statement* def) {
    if (pCond) pCond = pCond->expSubscriptVar(e, def);
    if (pDest) pDest = pDest->expSubscriptVar(e, def);
}


void BoolStatement::setDest(std::list<Statement*>* stmts) {
    assert(stmts->size() == 1);
    Assign* first = (Assign*)stmts->front();
    assert(first->getKind() == STMT_ASSIGN);
    pDest = first->getLeft();
}

//  //  //  //
// Assign //
//  //  //  //

Assign::Assign() {
    setKind(STMT_ASSIGN);
}
Assign::Assign(Exp* lhs, Exp* rhs) : lhs(lhs), rhs(rhs), type(NULL), guard(NULL)
{
    setKind(STMT_ASSIGN);
    if (lhs->getOper() == opTypedExp) { 
        type = ((TypedExp*)lhs)->getType(); 
    } 
    if (rhs->getOper() == opPhi)
        ((PhiExp*)rhs)->setStatement(this);
}

Assign::Assign(Type* ty, Exp* lhs, Exp* rhs) : lhs(lhs), rhs(rhs), type(ty),
  guard(NULL) 
{
    setKind(STMT_ASSIGN);
    if (rhs->getOper() == opPhi)
        ((PhiExp*)rhs)->setStatement(this);
}
Assign::Assign(Assign& o) {
    setKind(STMT_ASSIGN);
    lhs = o.lhs->clone();
    rhs = o.rhs->clone();
    if (o.type)  type  = o.type->clone();  else type  = NULL;
    if (o.guard) guard = o.guard->clone(); else guard = NULL;
}

Type* Assign::getType() {
    return type;
}
void Assign::setType(Type* ty) {
    type = ty;
}

Statement* Assign::clone() {
    Assign* a = new Assign(type == NULL ? NULL : type->clone(),
        lhs->clone(), rhs->clone());
    // Statement members
    a->pbb = pbb;
    a->proc = proc;
    a->number = number;
    return a;
}

// visit this Statement
bool Assign::accept(StmtVisitor* visitor) {
    return visitor->visit(this);
}

void Assign::simplify() {
    // simplify arithmetic of assignment
    if (Boomerang::get()->noBranchSimplify) {
        if (lhs->getOper() == opZF ||
            lhs->getOper() == opCF ||
            lhs->getOper() == opOF ||
            lhs->getOper() == opNF)
            return;
    }

    // this is a very complex pattern :)
    // replace:  1 r31 = a           where a is an array pointer
    //           2 r31 = phi{1 4}
    //           3 m[r31{2}] = x
    //           4 r31 = r31{2} + b  where b is the size of the base of the 
    //                               array pointed at by a
    // with:     1 r31 = 0
    //           2 r31 = phi{1 4}
    //           3 m[a][r31{2}] = x
    //           4 r31 = r31{2} + 1
    // I just assume this can only happen in a loop.. 
    if (lhs->getOper() == opMemOf && 
        lhs->getSubExp1()->getOper() == opSubscript &&
        ((RefExp*)lhs->getSubExp1())->getRef() &&
        ((RefExp*)lhs->getSubExp1())->getRef()->isPhi()) {
        Statement *phistmt = ((RefExp*)lhs->getSubExp1())->getRef();
        PhiExp *phi = (PhiExp*)phistmt->getRight();
        if (phi->getNumRefs() == 2 && 
            phi->getAt(0) && phi->getAt(1) &&
            phi->getAt(0)->isAssign() &&
            phi->getAt(1)->isAssign()) {
            Assign *a1 = (Assign*)phi->getAt(0);
            Assign *a4 = (Assign*)phi->getAt(1);
            if (a1->getRight()->getType() &&
                a4->getRight()->getOper() == opPlus &&
                a4->getRight()->getSubExp1()->getOper() == opSubscript &&
                ((RefExp*)a4->getRight()->getSubExp1())->getRef() == phistmt &&
                *a4->getRight()->getSubExp1()->getSubExp1() == 
                                                        *phi->getSubExp1() &&
                a4->getRight()->getSubExp2()->getOper() == opIntConst) {
                Type *ty = a1->getRight()->getType();
                int b = ((Const*)a4->getRight()->getSubExp2())->getInt();
                if (ty->resolvesToPointer()) {
                    ty = ty->asPointer()->getPointsTo();
                    if (ty->resolvesToArray() && 
                        b*8 == ty->asArray()->getBaseType()->getSize()) {
                        if (VERBOSE)
                            LOG << "doing complex pattern on " << this
                                << " using " << a1 << " and " << a4 << "\n";
                        ((Const*)a4->getRight()->getSubExp2())->setInt(1);
                        lhs = new Binary(opArraySubscript, 
                                Location::memOf(a1->getRight()->clone(), proc), 
                                lhs->getSubExp1()->clone());
                        a1->setRight(new Const(0));
                        if (VERBOSE)
                            LOG << "replaced with " << this << " using " 
                                << a1 << " and " << a4 << "\n";
                    }
                }
            }
        }
    }

    lhs = lhs->simplifyArith();
    rhs = rhs->simplifyArith();
    // simplify the resultant expression
    lhs = lhs->simplify();
    rhs = rhs->simplify();

    if (lhs->getOper() == opMemOf) {
        lhs->refSubExp1() = lhs->getSubExp1()->simplifyArith();
    }

    // this hack finds address constants.. it should go away when
    // Mike writes some decent type analysis.
    if (lhs->getOper() == opMemOf && 
        lhs->getSubExp1()->getOper() == opSubscript) {
        RefExp *ref = (RefExp*)lhs->getSubExp1();
        Statement *phist = ref->getRef();
        PhiExp *phi = NULL;
        if (phist && phist->getRight())
            phi = dynamic_cast<PhiExp*>(phist->getRight());
        for (int i = 0; phi && i < phi->getNumRefs(); i++) 
            if (phi->getAt(i)) {
                Assign *def = dynamic_cast<Assign*>(phi->getAt(i));
                if (def && (def->rhs->getOper() == opIntConst ||
                       (def->rhs->getOper() == opMinus &&
                        def->rhs->getSubExp1()->getOper() == opSubscript &&
                        ((RefExp*)def->rhs->getSubExp1())->getRef() == NULL &&
                        def->rhs->getSubExp1()->getSubExp1()->getOper() == 
                                                                    opRegOf &&
                        def->rhs->getSubExp2()->getOper() == opIntConst))) {
                    Exp *ne = new Unary(opAddrOf,
                        Location::memOf(def->rhs, proc)); 
                    if (VERBOSE)
                        LOG << "replacing " << def->rhs << " with " 
                            << ne << " in " << def << "\n";
                    def->rhs = ne;
                }
                if (def && def->rhs->getOper() == opAddrOf &&
                    def->rhs->getSubExp1()->getOper() == opSubscript &&
                    def->rhs->getSubExp1()->getSubExp1()->getOper() 
                                                      == opGlobal &&
                    rhs->getOper() != opPhi &&
                    rhs->getOper() != opItof &&
                    rhs->getOper() != opFltConst) {
                    Type *ty = proc->getProg()->getGlobalType(
                                 ((Const*)def->rhs->getSubExp1()->
                                                    getSubExp1()->
                                                    getSubExp1())->getStr());
                    if (ty && ty->isArray()) {
                        Type *bty = ((ArrayType*)ty)->getBaseType();
                        if (bty->isFloat()) {
                            if (VERBOSE)
                                LOG << "replacing " << rhs << " with ";
                            rhs = new Ternary(opItof, new Const(32), 
                                              new Const(bty->getSize()), rhs);
                            if (VERBOSE)
                                LOG << rhs 
                                    << " (assign indicates float type)\n";
                        }
                    }
                }
            }
    }

    // let's gather some more accurate type information
    if (lhs->isLocation() && rhs->getType()) {
        Location *llhs = dynamic_cast<Location*>(lhs);
        assert(llhs);
        Type *ty = rhs->getType();
        llhs->setType(ty);
        if (VERBOSE)
            LOG << "setting type of " << llhs << " to " << ty->getCtype() << "\n";
    }

    if (lhs->getType() && lhs->getType()->isFloat() && 
        rhs->getOper() == opIntConst) {
        if (lhs->getType()->getSize() == 32) {
            unsigned n = ((Const*)rhs)->getInt();
            rhs = new Const(*(float*)&n);
        }
    }

    if (lhs->getType() && lhs->getType()->isArray()) {
        lhs = new Binary(opArraySubscript, lhs, new Const(0));
    }
}

void Assign::simplifyAddr() {
    lhs = lhs->simplifyAddr();
    rhs = rhs->simplifyAddr();
}

void Assign::fixSuccessor() {
    lhs = lhs->fixSuccessor();
    rhs = rhs->fixSuccessor();
}

bool Assign::searchAndReplace(Exp* search, Exp* replace) {
    bool change = false;
    lhs = lhs->searchReplaceAll(search, replace, change);
    rhs = rhs->searchReplaceAll(search, replace, change);
    return change;
}

void Assign::print(std::ostream& os, bool withUses) {
    os << std::setw(4) << std::dec << number << " ";
    os << "*" << type << "* ";
    if (lhs) lhs->print(os, withUses);
    os << " := ";
    if (rhs) rhs->print(os, withUses);
}

void Assign::getDefinitions(LocationSet &defs) {
    defs.insert(lhs);
    // Special case: flag calls define %CF (and others)
    if (lhs->isFlags()) {
        defs.insert(new Terminal(opCF));
    }
    Location *loc = dynamic_cast<Location*>(lhs);
    if (loc)
        loc->getDefinitions(defs);
}

bool Assign::search(Exp* search, Exp*& result) {
    if (lhs->search(search, result))
        return true;
    return rhs->search(search, result);
}

bool Assign::searchAll(Exp* search, std::list<Exp*>& result) {
    bool res;
    std::list<Exp*> leftResult;
    std::list<Exp*>::iterator it;
    res = lhs->searchAll(search, leftResult);
    // Ugh: searchAll clears the list!
    res |= rhs->searchAll(search, result);
    for (it = leftResult.begin(); it != leftResult.end(); it++)
        result.push_back(*it);
    return res;
}

void Assign::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    hll->AddAssignmentStatement(indLevel, this);
}


int Assign::getMemDepth() {
    int d1 = lhs->getMemDepth();
    int d2 = rhs->getMemDepth();
	if (d1 >= d2) return d1;
	return d2;
}

void Assign::fromSSAform(igraph& ig) {
    lhs = lhs->fromSSAleft(ig, this);
    rhs = rhs->fromSSA(ig);
}

void Assign::setRight(Exp* e) {
    if (rhs) ;//delete rhs;
    rhs = e;
}

// update type for expression
Type *Assign::updateType(Exp *e, Type *curType) {
    return curType;
}

bool Assign::usesExp(Exp *e) {
    Exp *where = 0;
    return (rhs->search(e, where) || (lhs->isMemOf() && 
        ((Unary*)lhs)->getSubExp1()->search(e, where)));
}

bool Assign::doReplaceRef(Exp* from, Exp* to) {
    bool changeright = false;
    rhs = rhs->searchReplaceAll(from, to, changeright);
    bool changeleft = false;
    // If LHS is a memof, substitute its subexpression as well
    if (lhs->isMemOf()) {
        Exp* subsub1 = ((Unary*)lhs)->getSubExp1();
        ((Unary*)lhs)->setSubExp1ND(
          subsub1->searchReplaceAll(from, to, changeleft));
    }
    //assert(changeright || changeleft);    // HACK!
    if (!changeright && !changeleft) {
        // Could be propagating %flags into %CF
        Exp* baseFrom = ((RefExp*)from)->getSubExp1();
        if (baseFrom->isFlags()) {
            Statement* def = ((RefExp*)from)->getRef();
            Exp* defRhs = def->getRight();
            assert(defRhs->isFlagCall());
            /* When the carry flag is used bare, and was defined in a subtract
               of the form lhs - rhs, then CF has the value (lhs <u rhs)
               lhs and rhs are the first and second parameters of the flagcall
               Note: the flagcall is a binary, with a Const (the name) and a
               list of expressions:
                 defRhs
                 /    \
            Const      opList
            "SUBFLAGS"  /   \
                       P1   opList
                             /   \
                            P2  opList
                                 /   \
                                P3   opNil
            */
            Exp* e = new Binary(opLessUns,
                ((Binary*)defRhs)->getSubExp2()->getSubExp1(),
                ((Binary*)defRhs)->getSubExp2()->getSubExp2()->getSubExp1());
            rhs = rhs->searchReplaceAll(new RefExp(new Terminal(opCF), def),
              e, changeright);
        }
    }
    if (!changeright && !changeleft) {
        if (VERBOSE) {
            // I used to be hardline about this and assert fault,
            // but now I do such weird propagation orders that this can
            // happen.  It does however mean that some dataflow information
            // is wrong somewhere.  - trent
            LOG << "could not change " << from << " to " <<
              to << " in " << this << " !!\n";
        }
    }
    if (changeright) {
        // simplify the expression
        rhs = rhs->simplifyArith()->simplify();
    }
    if (changeleft) {
        lhs = lhs->simplifyArith()->simplify();
    }
    return false;
}

void Assign::subscriptVar(Exp* e, Statement* def) {
    // Replace all e with e{def} (on the RHS or in memofs in the LHS)
    // NOTE: don't use searchReplace. It deletes the original, which could
    // already be used as a key in a map!
    rhs = rhs->expSubscriptVar(e, def);
    if (lhs->isMemOf()) {
        Exp* subLeft = ((Unary*)lhs)->getSubExp1();
        Exp* temp = subLeft->expSubscriptVar(e, def);
        if (subLeft != temp)
            ((Unary*)lhs)->setSubExp1ND(temp);
    }
}

// Not sure if anything needed here
void Assign::processConstants(Prog* prog) {
#if 0
    LOG << "processing constants in assign lhs: " << lhs << " type: ";
    Type *ty = getTypeFor(lhs, prog);
    if (ty)
        LOG << ty->getCtype() << "\n";
    else
        LOG << "none\n";
#endif
    rhs = processConstant(rhs, getTypeFor(lhs, prog), prog);
}

// generate constraints
void Assign::genConstraints(LocationSet& cons) {
    Exp* con = rhs->genConstraints(
        new Unary(opTypeOf,
            new RefExp(lhs->clone(), this)));
    if (con) cons.insert(con);
}

void CallStatement::genConstraints(LocationSet& cons) {
    Proc* dest = getDestProc();
    if (dest == NULL) return;
    Signature* destSig = dest->getSignature();
    // Generate a constraint for the type of each actual argument to be equal
    // to the type of each formal parameter (hopefully, these are already
    // calculated correctly; if not, we need repeat till no change)
    int nPar = destSig->getNumParams();
    int min = 0;
#if 0
    if (dest->isLib())
        // Note: formals for a library signature start with the stack pointer
        min = 1;
#endif
    int a=0;        // Argument index
    for (int p=min; p < nPar; p++) {
        Exp* arg = arguments[a++];
        // Handle a[m[x]]
        if (arg->isAddrOf()) {
            Exp* sub = arg->getSubExp1();
            if (sub->isSubscript())
                sub = ((RefExp*)sub)->getSubExp1();
            if (sub->isMemOf())
                arg = ((Location*)sub)->getSubExp1();
        }
        if (arg->isRegOf() || arg->isMemOf() || arg->isSubscript() ||
              arg->isLocal() || arg->isGlobal()) {
            Exp* con = new Binary(opEquals,
                new Unary(opTypeOf, arg->clone()),
                new TypeVal(destSig->getParamType(p)->clone()));
            cons.insert(con);
        }
    }

    if (dest->isLib()) {
        // A library procedure... check for two special cases
        std::string name = dest->getName();
        // Note: might have to chase back via a phi statement to get a sample
        // string
        char* str;
        if ((name == "printf" || name == "scanf") &&
          (str = arguments[0]->getAnyStrConst()) != NULL) {
            // actually have to parse it
            int n = 1;      // Number of %s plus 1 = number of args
            char* p = str;
            while ((p = strchr(p, '%'))) {
                p++;
                Type* t = NULL;
                int longness = 0;
                bool sign = true;
                bool cont;
                do {
                    cont = false;
                    switch(*p) {
                        case 'u':
                            sign = false;
                            cont = true;
                            break;
                        case 'x':
                            sign = false;
                            // Fall through
                        case 'i':
                        case 'd': {
                            int size = 32;
                            // Note: the following only works for 32 bit code
                            // or where sizeof(long) == sizeof(int)
                            if (longness == 2) size = 64;
                            t = new IntegerType(size, sign);
                            break;
                        }
                        case 'f':
                        case 'g':
                            t = new FloatType(64);
                            break;
                        case 's':
                            t = new PointerType(new CharType());
                            break;
                        case 'l':
                            longness++;
                            cont = true;
                            break;
                        case '.':
                            cont = true;
                            break;
                        case '*':
                            assert(0);  // Star format not handled yet
                        default:
                            if (*p >= '0' && *p <= '9')
                                cont = true;
                            break;
                    }
                    p++;
                } while (cont);
                if (t) {
                    // scanf takes addresses of these
                    if (name == "scanf")
                        t = new PointerType(t);
                    // Generate a constraint for the parameter
                    TypeVal* tv = new TypeVal(t);
                    Exp* con = arguments[n]->genConstraints(tv);
                    cons.insert(con);
                }
                n++;
            }
        }
    }
}

void BranchStatement::genConstraints(LocationSet& cons) {
    if (pCond == NULL && VERBOSE) {
        LOG << "Warning: BranchStatment " << number <<
            " has no condition expression!\n";
        return;
    }
    assert(pCond->getArity() == 2);
    Exp* lhs = ((Binary*)pCond)->getSubExp1();
    Exp* rhs = ((Binary*)pCond)->getSubExp2();
    Exp* equ = new Binary(opEquals,
        new Unary(opTypeOf, lhs),
        new Unary(opTypeOf, rhs));
    cons.insert(equ);
}

int Statement::setConscripts(int n) {
    StmtSetConscripts ssc(n);
    accept(&ssc);
    return ssc.getLast();
}

bool Statement::stripRefs() {
    StripRefs sr;
    StripPhis sp(&sr);
    accept(&sp);
    return sp.getDelete();
}

// Visiting from class StmtExpVisitor
// Visit all the various expressions in a statement
bool Assign::accept(StmtExpVisitor* v) {
    bool override;
    bool ret = v->visit(this, override);
    if (override)
        // The visitor has overridden this functionality
        // This is needed for example in UsedLocFinder, where the lhs of an
        // assignment is not used (but it it's m[blah], then blah is used)
        return ret;
    if (ret && lhs) ret = lhs->accept(v->ev);
    if (ret && rhs) ret = rhs->accept(v->ev);
    return ret;
}

bool GotoStatement::accept(StmtExpVisitor* v) {
    bool override;
    bool ret = v->visit(this, override);
    if (override) return ret;
    if (ret && pDest)
        ret = pDest->accept(v->ev);
    return ret;
}

bool BranchStatement::accept(StmtExpVisitor* v) {
    bool override;
    bool ret = v->visit(this, override);
    if (override) return ret;
    // Destination will always be a const for X86, so the below will never
    // be used in practice
    if (ret && pDest)
        ret = pDest->accept(v->ev);
    if (ret && pCond)
        ret = pCond->accept(v->ev);
    return ret;
}

bool CaseStatement::accept(StmtExpVisitor* v) {
    bool override;
    bool ret = v->visit(this, override);
    if (override) return ret;
    if (ret && pDest)
        ret = pDest->accept(v->ev);
    if (ret && pSwitchInfo && pSwitchInfo->pSwitchVar)
        ret = pSwitchInfo->pSwitchVar->accept(v->ev);
    return ret;
}

bool CallStatement::accept(StmtExpVisitor* v) {
    bool override;
    bool ret = v->visit(this, override);
    if (override) return ret;
    if (ret && pDest)
        ret = pDest->accept(v->ev);
    std::vector<Exp*>::iterator it;
    for (it = arguments.begin(); ret && it != arguments.end(); it++)
        ret = (*it)->accept(v->ev);
    for (it = implicitArguments.begin(); ret && it != implicitArguments.end(); it++)
        ret = (*it)->accept(v->ev);
    for (it = returns.begin(); ret && it != returns.end(); it++)
        ret = (*it)->accept(v->ev);
    return ret;
}

bool ReturnStatement::accept(StmtExpVisitor* v) {
    bool override;
    bool ret = v->visit(this, override);
    if (override) return ret;
    std::vector<Exp*>::iterator it;
    for (it = returns.begin(); ret && it != returns.end(); it++)
        ret = (*it)->accept(v->ev);
    return ret;
}

bool BoolStatement::accept(StmtExpVisitor* v) {
    bool override;
    bool ret = v->visit(this, override);
    if (override) return ret;
    if (ret && pCond)
        ret = pCond->accept(v->ev);
    return ret;
}

// Visiting from class StmtModifier
// Modify all the various expressions in a statement
bool Assign::accept(StmtModifier* v) {
    v->visit(this);
    v->mod->clearMod();
    if (lhs) lhs = lhs->accept(v->mod);
    if (rhs) rhs = rhs->accept(v->mod);
    if (VERBOSE && v->mod->isMod())
        LOG << "Assignment changed: now " << this << "\n";
    return true;
}

bool GotoStatement::accept(StmtModifier* v) {
    v->visit(this);
    if (pDest)
        pDest = pDest->accept(v->mod);
    return true;
}

bool BranchStatement::accept(StmtModifier* v) {
    v->visit(this);
    if (pDest)
        pDest = pDest->accept(v->mod);
    return true;
}

bool CaseStatement::accept(StmtModifier* v) {
    v->visit(this);
    if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->accept(v->mod);
    return true;
}

bool CallStatement::accept(StmtModifier* v) {
    v->visit(this);
    if (pDest)
        pDest = pDest->accept(v->mod);
    std::vector<Exp*>::iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++)
        *it = (*it)->accept(v->mod);
    for (it = implicitArguments.begin(); it != implicitArguments.end(); it++)
        *it = (*it)->accept(v->mod);
    for (it = returns.begin(); it != returns.end(); it++)
        *it = (*it)->accept(v->mod);
    return true;
}

bool ReturnStatement::accept(StmtModifier* v) {
    v->visit(this);
    std::vector<Exp*>::iterator it;
    for (it = returns.begin(); it != returns.end(); it++)
        *it = (*it)->accept(v->mod);
    return true;
}

bool BoolStatement::accept(StmtModifier* v) {
    v->visit(this);
    if (pCond)
        pCond = pCond->accept(v->mod);
    return true;
}

void Statement::fixCallRefs() {
    CallRefsFixer crf;
    StmtModifier sm(&crf);
    accept(&sm);
}

// Find the locations used by expressions in this Statement.
// Use the StmtExpVisitor and UsedLocsFinder visitor classes
void Statement::addUsedLocs(LocationSet& used, bool final /* = false */) {
    UsedLocsFinder ulf(used);
    UsedLocsVisitor ulv(&ulf, final);
    accept(&ulv);
}
