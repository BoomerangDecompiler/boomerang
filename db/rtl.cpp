/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       rtl.cc
 * OVERVIEW:   Implementation of the classes that describe a low level RTL (
 *             register transfer list)
 *============================================================================*/

/*
 * $Revision$
 * 
 * 08 Apr 02 - Mike: Changes for boomerang
 * 13 May 02 - Mike: expList is no longer a pointer
 * 15 May 02 - Mike: Fixed a nasty bug in updateExp (when update with same
 *              expression as existing)
 * 25 Jul 02 - Mike: RTL is now list of Statements
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <iomanip>          // For setfill
#include <sstream>
#include "types.h"
#include "statement.h"
#include "exp.h"
#include "type.h"
#include "register.h"
#include "cfg.h"
#include "proc.h"           // For printing proc names
#include "rtl.h"
#include "prog.h"
#include "hllcode.h"
#include "util.h"
#include "boomerang.h"
#include "visitor.h"
// For some reason, MSVC 5.00 complains about use of undefined types a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"		// For MSVC 5.00
#endif

/******************************************************************************
 * RTL methods.
 * Class RTL represents low-level register transfer lists. 
 *****************************************************************************/

/*==============================================================================
 * FUNCTION:        RTL::RTL
 * OVERVIEW:        Constructor.
 * PARAMETERS:      <none>
 * RETURNS:         N/a
 *============================================================================*/
RTL::RTL()
  : nativeAddr(0)
{ }

/*==============================================================================
 * FUNCTION:        RTL::RTL
 * OVERVIEW:        Constructor.
 * PARAMETERS:      instNativeAddr - the native address of the instruction
 *                  listExp - ptr to existing list of Exps
 * RETURNS:         N/a
 *============================================================================*/
RTL::RTL(ADDRESS instNativeAddr, std::list<Statement*>* listStmt /*= NULL*/)
    : nativeAddr(instNativeAddr) {
    if (listStmt)
        stmtList = *listStmt;
}

/*==============================================================================
 * FUNCTION:        RTL::RTL
 * OVERVIEW:        Copy constructor. A deep clone is made of the given object
 *                  so that the lists of Exps do not share memory.
 * PARAMETERS:      other: RTL to copy from
 * RETURNS:         N/a
 *============================================================================*/
RTL::RTL(const RTL& other)
  : nativeAddr(other.nativeAddr) {
    std::list<Statement*>::const_iterator it;
    for (it = other.stmtList.begin(); it != other.stmtList.end(); it++) {
        stmtList.push_back((*it)->clone());
    }
}

/*==============================================================================
 * FUNCTION:        RTL::~RTL
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         N/a
 *============================================================================*/
RTL::~RTL() {
    std::list<Statement*>::iterator it;
    for (it = stmtList.begin(); it != stmtList.end(); it++) {
        if (*it != NULL) {
            //delete *it;
        }
    }
}

/*==============================================================================
 * FUNCTION:        RTL::operator=
 * OVERVIEW:        Assignment copy (deep).
 * PARAMETERS:      other - RTL to copy
 * RETURNS:         a reference to this object
 *============================================================================*/
RTL& RTL::operator=(RTL& other) {
    if (this != &other) {
        // Do a deep copy always
        std::list<Statement*>::iterator it;
        for (it = other.stmtList.begin(); it != other.stmtList.end(); it++)
            stmtList.push_back((*it)->clone());
        
        nativeAddr = other.nativeAddr;
    }
    return *this;
}

/*==============================================================================
 * FUNCTION:        RTL:clone
 * OVERVIEW:        Deep copy clone; deleting the clone will not affect this
 *                   RTL object
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this one
 *============================================================================*/
RTL* RTL::clone() {
    std::list<Statement*> le;
    std::list<Statement*>::iterator it;

    for (it = stmtList.begin(); it != stmtList.end(); it++) {
        le.push_back((*it)->clone());
    }
    
    RTL* ret = new RTL(nativeAddr, &le);
    return ret;
}

// visit this RTL, and all its Statements
bool RTL::accept(StmtVisitor* visitor) {
    // Might want to do something at the RTL level:
    if (!visitor->visit(this)) return false;
    std::list<Statement*>::iterator it;
    for (it = stmtList.begin(); it != stmtList.end(); it++) {
        if (! (*it)->accept(visitor)) return false;
    }
    return true;
}

/*==============================================================================
 * FUNCTION:        RTL::deepCopyList
 * OVERVIEW:        Make a copy of this RTLs list of Exp* to the given list
 * PARAMETERS:      Ref to empty list to copy to
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::deepCopyList(std::list<Statement*>& dest) {
    std::list<Statement*>::iterator it;

    for (it = stmtList.begin(); it != stmtList.end(); it++) {
        dest.push_back((*it)->clone());
    }
}

/*==============================================================================
 * FUNCTION:        RTL::appendStmt
 * OVERVIEW:        Append the given Statement at the end of this RTL
 * NOTE:            Exception: Leaves any flag call at the end (so may push exp
 *                   to second last position, instead of last)
 * NOTE:            stmt is NOT copied. This is different to how UQBT was!
 * PARAMETERS:      s: pointer to Statement to append
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::appendStmt(Statement* s) {
    if (stmtList.size()) {
        if (stmtList.back()->isFlagAssgn()) {
            std::list<Statement*>::iterator it = stmtList.end();
            stmtList.insert(--it, s);
            return;
        }
    }
    stmtList.push_back(s);
}

/*==============================================================================
 * FUNCTION:        RTL::prependStmt
 * OVERVIEW:        Prepend the given Statement at the start of this RTL
 * NOTE:            No copy of the statement is made. This is different to how
 *                    UQBT was!
 * PARAMETERS:      s: Ptr to Statement to prepend
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::prependStmt(Statement* s) {
    stmtList.push_front(s);
}

/*==============================================================================
 * FUNCTION:        RTL::appendListStmt
 * OVERVIEW:        Append a given list of Statements to this RTL
 * NOTE:            A copy of the Statements in le are appended
 * PARAMETERS:      rtl: list of Exps to insert
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::appendListStmt(std::list<Statement*>& le) {
    std::list<Statement*>::iterator it;
    for (it = le.begin();  it != le.end();  it++) {
        stmtList.insert(stmtList.end(), (*it)->clone());
    }
}

/*==============================================================================
 * FUNCTION:        RTL::appendRTL
 * OVERVIEW:        Append the Statemens of another RTL to this object
 * NOTE:            A copy of the Statements in r are appended
 * PARAMETERS:      r: reterence to RTL whose Exps we are to insert
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::appendRTL(RTL& r) {
    appendListStmt(r.stmtList);
}

/*==============================================================================
 * FUNCTION:        RTL::insertStmt
 * OVERVIEW:        Insert the given Statement before index i
 * NOTE:            No copy of stmt is made. This is different to UQBT
 * PARAMETERS:      s: pointer to the Statement to insert
 *                  i: position to insert before (0 = first)
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::insertStmt(Statement* s, unsigned i) {
    // Check that position i is not out of bounds
    assert (i < stmtList.size() || stmtList.size() == 0);

    // Find the position
    std::list<Statement*>::iterator pp = stmtList.begin();
    for (; i > 0; i--, pp++);

    // Do the insertion
    stmtList.insert(pp, s);
}

/*==============================================================================
 * FUNCTION:        RTL::updateStmt
 * OVERVIEW:        Replace the ith Statement with the given one
 * PARAMETERS:      s: pointer to the new Exp
 *                  i: index of Exp position (0 = first)
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::updateStmt(Statement *s, unsigned i) {
    // Check that position i is not out of bounds
    assert (i < stmtList.size());

    // Find the position
    std::list<Statement*>::iterator pp = stmtList.begin();
    for (; i > 0; i--, pp++);    

    // Note that sometimes we might update even when we don't know if it's
    // needed, e.g. after a searchReplace.
    // In that case, don't update, and especially don't delete the existing
    // statement (because it's also the one we are updating!)
    if (*pp != s) {
        // Do the update
        if (*pp) ;//delete *pp;
        *pp = s;
    }
}

void RTL::deleteStmt(unsigned i) {
    // check that position i is not out of bounds
    assert (i < stmtList.size());

    // find the position
    std::list<Statement*>::iterator pp = stmtList.begin();
    for (; i > 0; i--, pp++);    

    // do the delete
    stmtList.erase(pp);
}
    
/*==============================================================================
 * FUNCTION:        RTL::getNumStmt
 * OVERVIEW:        Get the number of Statements in this RTL
 * PARAMETERS:      None
 * RETURNS:         Integer number of Statements
 *============================================================================*/
int RTL::getNumStmt() {
    return stmtList.size();
}

/*==============================================================================
 * FUNCTION:        RTL::at
 * OVERVIEW:        Provides indexing on a list. Changed from operator[] so that
 *                  we keep in mind it is linear in its execution time.
 * PARAMETERS:      i - the index of the element we want (0 = first)
 * RETURNS:         the element at the given index or NULL if the index is out
 *                  of bounds
 *============================================================================*/
Statement* RTL::elementAt(unsigned i) {
    std::list<Statement*>::iterator it;
    for (it = stmtList.begin();  i > 0 && it != stmtList.end();  i--, it++);
    if (it == stmtList.end()) {
        return NULL;
    }
    return *it;
}

/*==============================================================================
 * FUNCTION:        RTL::print
 * OVERVIEW:        Prints this object to a stream in text form.
 * PARAMETERS:      os - stream to output to (often cout or cerr)
 * RETURNS:         <nothing>
 *============================================================================*/
void RTL::print(std::ostream& os /*= cout*/, bool withDF /*= false*/) {

    // print out the instruction address of this RTL
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << std::dec << std::setfill(' ');      // Ugh - why is this needed?

    // Print the statements
    // First line has 8 extra chars as above
    bool bFirst = true;
    std::list<Statement*>::iterator ss;
    for (ss = stmtList.begin(); ss != stmtList.end(); ss++) {
        Statement* stmt = *ss;
        if (bFirst) os << " ";
        else        os << std::setw(9) << " ";
        if (stmt) {
            if (withDF)
                stmt->printWithUses(os);
            else
                stmt->print(os);
        }
        // Note: we only put newlines where needed. So none at the end of
        // Statement::print; one here to separate from other statements
        os << "\n";
        bFirst = false;
    }
    if (stmtList.empty()) os << std::endl;     // New line for NOP
}

extern char debug_buffer[];
char* RTL::prints() {
      std::ostringstream ost;
      print(ost, true);
      strncpy(debug_buffer, ost.str().c_str(), 199);
      debug_buffer[199] = '\0';
      return debug_buffer;
}

/*==============================================================================
 * FUNCTION:        RTL::getAddress
 * OVERVIEW:        Return the native address of this RTL
 * PARAMETERS:      None
 * RETURNS:         Native address
 *============================================================================*/
ADDRESS RTL::getAddress() {
    return nativeAddr;
}


/*==============================================================================
 * FUNCTION:        RTL::updateAddress
 * OVERVIEW:        Set the nativeAddr field
 * PARAMETERS:      Native address
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::updateAddress(ADDRESS addr) {
    nativeAddr = addr;
}

/*==============================================================================
 * FUNCTION:        RTL::searchReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - ptr to an expression to search for
 *                  replace - ptr to the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
bool RTL::searchAndReplace(Exp* search, Exp* replace) {
    bool ch = false;
    for (std::list<Statement*>::iterator it = stmtList.begin();
      it != stmtList.end(); it++)
        ch |= (*it)->searchAndReplace(search, replace);
    return ch;
}

/*==============================================================================
 * FUNCTION:        RTL::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool RTL::searchAll(Exp* search, std::list<Exp *> &result) {
    bool found = false;
    for (std::list<Statement*>::iterator it = stmtList.begin();
      it != stmtList.end(); it++) {
        Statement *e = *it;
        Exp* res;
        if (e->search(search, res)) {
            found = true;
            result.push_back(res);
        }
    }
    return found;
}

/*==============================================================================
 * FUNCTION:        RTL::clear
 * OVERVIEW:        Clear the list of Exps
 * PARAMETERS:      None
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::clear() {
    stmtList.clear();
}

/*==============================================================================
 * FUNCTION:        RTL::insertAssign
 * OVERVIEW:        Prepends or appends an assignment to the front or back of
 *                    this RTL
 * NOTE:            Is this really used? What about types?
 * ASSUMES:         Assumes that pLhs and pRhs are "new" Exp's that are
 *                  not part of other Exps. (Otherwise, there will be problems
 *                  when deleting this Exp)
 *                  If size == -1, assumes there is already at least one assign-
 *                    ment in this RTL
 * PARAMETERS:      pLhs: ptr to Exp to place on LHS
 *                  pRhs: ptr to Exp to place on the RHS
 *                  prep: true if prepend (else append)
 *                  type: type of the transfer, or NULL
 * RETURNS:         <nothing>
 *============================================================================*/
void RTL::insertAssign(Exp* pLhs, Exp* pRhs, bool prep,
                        Type* type /*= NULL */) {
    // Generate the assignment expression
    Assign* asgn = new Assign(type, pLhs, pRhs);
    if (prep)
        prependStmt(asgn);
    else
        appendStmt(asgn);
}

/*==============================================================================
 * FUNCTION:        RTL::insertAfterTemps
 * OVERVIEW:        Inserts an assignment at or near the top of this RTL, after
 *                    any assignments to temporaries. If the last assignment
 *                    is to a temp, the insertion is done before that last
 *                    assignment
 * ASSUMES:         Assumes that ssLhs and ssRhs are "new" Exp's that are
 *                  not part of other Exps. (Otherwise, there will be problems
 *                  when deleting this Exp)
 *                  If type == NULL, assumes there is already at least one
 *                    assignment in this RTL (?)
 * NOTE:            Hopefully this is only a temporary measure
 * PARAMETERS:      pLhs: ptr to Exp to place on LHS
 *                  pRhs: ptr to Exp to place on the RHS
 *                  size: size of the transfer, or -1 to be the same as the
 *                    first assign this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void RTL::insertAfterTemps(Exp* pLhs, Exp* pRhs, Type* type  /* NULL */) {
    std::list<Statement*>::iterator it;
    // First skip all assignments with temps on LHS
    for (it = stmtList.begin(); it != stmtList.end(); it++) {
    Statement *e = *it;
        if (!e->isAssign())
            break;
        Exp* LHS = e->getLeft();
        if (LHS->isTemp())
            break;
    }

    // Now check if the next Stmt is an assignment
    if ((it == stmtList.end()) || !(*it)->isAssign()) {
        // There isn't an assignment following. Use the previous Exp to insert
        // before
        if (it != stmtList.begin())
            it--;
    }

    if (type == NULL)
        type = getType();

    // Generate the assignment expression
    Assign* asgn = new Assign(type, pLhs, pRhs);

    // Insert before "it"
    stmtList.insert(it, asgn);
}

/*==============================================================================
 * FUNCTION:        RTL::getType
 * OVERVIEW:        Get the "type" for this RTL. Just gets the type of
 *                    the first assignment Exp
 * NOTE:            The type of the first assign may not be the type that you
 *                    want!
 * PARAMETERS:      None
 * RETURNS:         A pointer to the type
 *============================================================================*/
Type* RTL::getType() {
    std::list<Statement*>::iterator it;
    for (it = stmtList.begin(); it != stmtList.end(); it++) {
        Statement *e = *it;
        if (e->isAssign())
            return ((Assign*)e)->getType();
    }
    return new IntegerType();   //  Default to 32 bit integer if no assignments
}

/*==============================================================================
 * FUNCTION:      RTL::areFlagsAffected
 * OVERVIEW:      Return true if this RTL affects the condition codes
 * NOTE:          Assumes that if there is a flag call Exp, then it is the last
 * PARAMETERS:    None
 * RETURNS:       Boolean as above
 *============================================================================*/
bool RTL::areFlagsAffected() {
    if (stmtList.size() == 0) return false;
    // Get an iterator to the last RT
    std::list<Statement*>::iterator it = stmtList.end();
    if (it == stmtList.begin())
        return false;           // Not expressions at all
    it--;                       // Will now point to the end of the list
    Statement *e = *it;
    // If it is a flag call, then the CCs are affected
    return e->isFlagAssgn();
}

void RTL::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    for (std::list<Statement*>::iterator it = stmtList.begin();
      it != stmtList.end(); it++) {
        (*it)->generateCode(hll, pbb, indLevel);
    }
}

void RTL::simplify() {
    for (std::list<Statement*>::iterator it = stmtList.begin();
         it != stmtList.end(); /*it++*/) {
        Statement *s = *it;
        s->simplify();        
        if (s->isBranch()) {
            Exp *cond =  ((BranchStatement*)s)->getCondExpr();
            if (cond->getOper() == opIntConst) {
                if (((Const*)cond)->getInt() == 0) {
					if (VERBOSE)
						LOG << "removing branch with false condition at " << getAddress()  << " " << *it << "\n";
                    it = stmtList.erase(it);
                    continue;
                } else {
					if (VERBOSE)
						LOG << "replacing branch with true condition with goto at " << getAddress() << " " << *it << "\n";
                    *it = new GotoStatement(((BranchStatement*)s)->getFixedDest());
                }
            }
        }
        it++;
    }
}

/*==============================================================================
 * FUNCTION:        RTL::isCompare
 * OVERVIEW:        Return true if this is an unmodified compare instruction
 *                    of a register with an operand
 * NOTE:            Will also match a subtract if the flags are set
 * NOTE:            expOperand, if set, is not cloned
 * NOTE:            Assumes that the first subtract on the RHS is the actual
 *                    compare operation
 * PARAMETERS:      iReg: ref to integer to set with the register index
 *                  expOperand: ref to ptr to expression of operand
 * RETURNS:         True if found
 *============================================================================*/
bool RTL::isCompare(int& iReg, Exp*& expOperand) {
    // Expect to see a subtract, then a setting of the flags
    // Dest of subtract should be a register (could be the always zero register)
    if (getNumStmt() < 2) return false;
    // Could be first some assignments to temporaries
    // But the actual compare could also be an assignment to a temporary
    // So we search for the first RHS with an opMinus, that has a LHS to
    // a register (whether a temporary or a machine register)
    int i=0;
    Exp* rhs;
    Statement* cur;
    do {
        cur = elementAt(i);
        if (cur->getKind() != STMT_ASSIGN) return false;
        rhs = cur->getRight();
        i++;
    } while (rhs->getOper() != opMinus && i < getNumStmt());
    if (rhs->getOper() != opMinus) return false;
    // We should be rid of all r[tmp] now...
      // Exp* lhs = cur->getLeft();
      // if (!lhs->isRegOf()) return false;
    // We have a subtract assigning to a register.
    // Check if there is a subflags last
    Statement* last = elementAt(getNumStmt()-1);
    if (!last->isFlagAssgn()) return false;
    Exp* sub = ((Binary*)rhs)->getSubExp1();
    // Should be a compare of a register and something (probably a constant)
    if (!sub->isRegOf()) return false;
    // Set the register and operand expression, and return true
    iReg = ((Const*)((Unary*)sub)->getSubExp1())->getInt();
    expOperand = ((Binary*)rhs)->getSubExp2();
    return true;
}

bool RTL::isGoto() {
    if (stmtList.empty()) return false;
    Statement* last = stmtList.back();
    return last->getKind() == STMT_GOTO;
}

bool RTL::isBranch() {
    if (stmtList.empty()) return false;
    Statement* last = stmtList.back();
    return last->getKind() == STMT_BRANCH;
}

bool RTL::isCall() {
    if (stmtList.empty()) return false;
    Statement* last = stmtList.back();
    return last->getKind() == STMT_CALL;
}

// Use this slow function when you can't be sure that the HL Statement is last
Statement* RTL::getHlStmt() {
    std::list<Statement*>::reverse_iterator rit;
    for (rit = stmtList.rbegin(); rit != stmtList.rend(); rit++) {
        if ((*rit)->getKind() != STMT_ASSIGN)
            return *rit;
    }
    return NULL;
}

int RTL::setConscripts(int n) {
    StmtSetConscripts ssc(n);
    accept(&ssc);
    return ssc.getLast();
}
