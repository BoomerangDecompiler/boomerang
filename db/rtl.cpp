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
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <iomanip>          // For setfill
#include <sstream>
#include "types.h"
#include "exp.h"
#include "type.h"
#include "register.h"
#include "dataflow.h"
#include "proc.h"           // For printing proc names
#include "rtl.h"
#include "prog.h"
#include "hllcode.h"
#include "util.h"

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
    : kind(HL_NONE), nativeAddr(0), numNativeBytes(0), isCommented(false)
{ }

/*==============================================================================
 * FUNCTION:        RTL::RTL
 * OVERVIEW:        Constructor.
 * PARAMETERS:      instNativeAddr - the native address of the instruction
 *                  listExp - ptr to existing list of Exps
 * RETURNS:         N/a
 *============================================================================*/
RTL::RTL(ADDRESS instNativeAddr, std::list<Exp*>* listExp /*= NULL*/)
    : kind(HL_NONE), nativeAddr(instNativeAddr), numNativeBytes(0),
      isCommented(false) {
    if (listExp)
        expList = *listExp;
}

/*==============================================================================
 * FUNCTION:        RTL::RTL
 * OVERVIEW:        Copy constructor. A deep clone is made of the given object
 *                  so that the lists of Exps do not share memory.
 * PARAMETERS:      other: RTL to copy from
 * RETURNS:         N/a
 *============================================================================*/
RTL::RTL(const RTL& other)
    : kind(other.kind), nativeAddr(other.nativeAddr),
      numNativeBytes(other.numNativeBytes), isCommented(other.isCommented)
{
    std::list<Exp*>::const_iterator it;
    for (it = other.expList.begin(); it != other.expList.end(); it++) {
        expList.push_back((*it)->clone());
    }
}

/*==============================================================================
 * FUNCTION:        RTL::~RTL
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         N/a
 *============================================================================*/
RTL::~RTL() {
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++) {
        if (*it != NULL) {
            delete *it;
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
        std::list<Exp*>::iterator it;
        for (it = other.expList.begin(); it != other.expList.end(); it++) {
            Exp* e = (*it)->clone();
            expList.push_back(e);
        }
        
        kind = other.kind;
        nativeAddr = other.nativeAddr;
        numNativeBytes = other.numNativeBytes;
        isCommented = other.isCommented;
    }
    return *this;
}

// visit this rtl
bool RTL::accept(RTLVisitor* visitor) {
    return visitor->visit(this);
}

/*==============================================================================
 * FUNCTION:        RTL:clone
 * OVERVIEW:        Deep copy clone; deleting the clone will not affect this
 *                   RTL object
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this one
 *============================================================================*/
RTL* RTL::clone() {
    std::list<Exp*> le;
    std::list<Exp*>::iterator it;

    for (it = expList.begin(); it != expList.end(); it++) {
        le.push_back((*it)->clone());
    }
    
    RTL* ret = new RTL(nativeAddr, &le);
    ret->kind = kind;
    ret->numNativeBytes = numNativeBytes;
    ret->isCommented = isCommented;
    return ret;
}

/*==============================================================================
 * FUNCTION:        RTL::deepCopyList
 * OVERVIEW:        Make a copy of this RTLs list of Exp* to the given list
 * PARAMETERS:      Ref to empty list to copy to
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::deepCopyList(std::list<Exp*>& dest) {
    std::list<Exp*>::iterator it;

    for (it = expList.begin(); it != expList.end(); it++) {
        dest.push_back((*it)->clone());
    }
}

/*==============================================================================
 * FUNCTION:        RTL::appendExp
 * OVERVIEW:        Append the given Exp at the end of this RTL
 * NOTE:            Exception: Leaves any flag call at the end (so may push exp
 *                   to second last position, instead of last)
 * NOTE:            exp is NOT copied. This is different to how UQBT was!
 * PARAMETERS:      rt: pointer to Exp to append
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::appendExp(Exp* exp) {
    if (expList.size() && (expList.back()->isFlagCall())) {
        std::list<Exp*>::iterator it = expList.end();
        expList.insert(--it, exp);
    } else {
        expList.push_back(exp);
    }
}

/*==============================================================================
 * FUNCTION:        RTL::prependExp
 * OVERVIEW:        Prepend the given Exp at the start of this RTL
 * NOTE:            No copy of exp is made. This is different to how UQBT was!
 * PARAMETERS:      rtxp to Exp to prepend
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::prependExp(Exp* exp) {
    expList.push_front(exp);
}

/*==============================================================================
 * FUNCTION:        RTL::appendListExp
 * OVERVIEW:        Append a given list of Exp*s to this RTL
 * NOTE:            A copy of the Exps in le are appended
 * PARAMETERS:      rtl: list of Exps to insert
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::appendListExp(std::list<Exp*>& le) {
    std::list<Exp*>::iterator it;
    for (it = le.begin();  it != le.end();  it++) {
        expList.insert(expList.end(), (*it)->clone());
    }
}

/*==============================================================================
 * FUNCTION:        RTL::appendExplist
 * OVERVIEW:        Append the Exps of another RTL to this object
 * NOTE:            A copy of the Exps in h are appended
 * PARAMETERS:      rtl: RTL whose Exps we are to insert
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::appendRTL(RTL& r) {
    appendListExp(r.expList);
}

/*==============================================================================
 * FUNCTION:        RTL::insertExp
 * OVERVIEW:        Insert the given Exp before index i
 * NOTE:            No copy of exp is made. This is different to UQBT
 * PARAMETERS:      exp: pointer to the Exp to insert
 *                  i: position to insert before (0 = first)
 * RETURNS:         Nothing
 *============================================================================*/
// Insert register transfer at position i (or the head of the list)
void RTL::insertExp(Exp* exp, unsigned i) {
    // Check that position i is not out of bounds
    assert (i < expList.size() || expList.size() == 0);

    // Find the position
    std::list<Exp*>::iterator pp = expList.begin();
    for (; i > 0; i--, pp++);

    // Do the insertion
    expList.insert(pp, exp);
}

/*==============================================================================
 * FUNCTION:        RTL::updateExp
 * OVERVIEW:        Replace the ith Exp with the given Exp
 * PARAMETERS:      exp: pointer to the new Exp
 *                  i: index of Exp position (0 = first)
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::updateExp(Exp *exp, unsigned i) {
    // Check that position i is not out of bounds
    assert (i < expList.size());

    // Find the position
    std::list<Exp*>::iterator pp = expList.begin();
    for (; i > 0; i--, pp++);    

    // Note that sometimes we might update even when we don't know if it's
    // needed, e.g. after a searchReplace.
    // In that case, don't update, and especially don't delete the existing
    // expression (because it's also the one we are updating!)
    if (!((char*)*pp == (char*)exp)) {
        // Do the update
        if (*pp) delete *pp;
        *pp = exp;
    }
}

void RTL::deleteExp(unsigned i) {
    // check that position i is not out of bounds
    assert (i < expList.size());

    // find the position
    std::list<Exp*>::iterator pp = expList.begin();
    for (; i > 0; i--, pp++);    

    // do the delete
    expList.erase(pp);
}
    
/*==============================================================================
 * FUNCTION:        RTL::getNumExp
 * OVERVIEW:        Get the number of Exps in this RTL
 * PARAMETERS:      None
 * RETURNS:         Integer number of Exps
 *============================================================================*/
int RTL::getNumExp() {
    return expList.size();
}

/*==============================================================================
 * FUNCTION:        RTL::at
 * OVERVIEW:        Provides indexing on a list. Changed from operator[] so that
 *                  we keep in mind it is linear in its execution time.
 * PARAMETERS:      i - the index of the element we want
 * RETURNS:         the element at the given index or NULL if the index is out
 *                  of bounds
 *============================================================================*/
Exp* RTL::elementAt(unsigned i) {
    std::list<Exp*>::iterator it;
    for (it = expList.begin();  i > 0 && it != expList.end();  i--, it++);
    if (it == expList.end()) {
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
void RTL::print(std::ostream& os /*= cout*/) {

    // print out the instruction address of this RTL
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << std::dec << std::setfill(' ');      // Ugh - why is this needed?

    // Print the register transfers
    // First line has 8 extra chars as above
    bool bFirst = true;
    std::list<Exp*>::iterator p;
    for (p = expList.begin(); p != expList.end(); p++)
    {
        if (bFirst) os << " ";
        else        os << std::setw(9) << " ";
        (*p)->print(os);
        os << "\n";
        bFirst = false;
    }
    if (expList.empty()) os << std::endl;     // New line for NOP
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
 * FUNCTION:        RTL::setCommented
 * OVERVIEW:        Set the isCommented flag (so this RTL will be emitted as a
 *                    comment)
 * PARAMETERS:      state: whether to set or reset the flag
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::setCommented(bool state) {
    isCommented = state;
}

/*==============================================================================
 * FUNCTION:        RTL::getCommented
 * OVERVIEW:        Get the isCommented flag
 * PARAMETERS:      state: whether to set or reset the flag
 * RETURNS:         Nothing
 *============================================================================*/
bool RTL::getCommented() {
    return isCommented;
}

/*==============================================================================
 * FUNCTION:        RTL::expSubAXP
 * OVERVIEW:        (See comment for Proc::subAXP)
 * NOTE:            Was in RTAssgn::subAXP; assumes a typed assignment expr
 * PARAMETERS:      exp: ref to Exp to substitute (could be an assignment or
 *                    a flag call)
 *                  subMap - a map from expression to expression
 * RETURNS:         True if "left hand side" is substituted  (CHECK THIS!)
 *============================================================================*/
bool expSubAXP(Exp* exp, std::map<Exp*, Exp*>& subMap)
{
    // Record whether or not this assignment is a definition of a register
    // being substituted
    bool isDef = false;
    bool change;
    Exp* pLHS = exp->getSubExp1()->getSubExp1();
    Exp* pRHS = exp->getSubExp1()->getSubExp2();

    // Go through each entry in the substitution map
    for (std::map<Exp*,Exp*>::iterator it = subMap.begin();
      it != subMap.end(); it++) {

        // Replace any uses of the current register with its
        // corresponding substitution
        if (pRHS = pRHS->searchReplaceAll(it->first, it->second, change),
          change) {
            // Remove any sizes and sign extends; these will just complicate
            // simplification. Address expressions shouldn't have these anyway
            // No! This actually hacks off the size and sign casts from the
            // whole expression (not just the address part)
            // Commenting out the below may break some Palm code. But another
            // way has to be found!
//          pRHS->removeSize();
            pRHS->simplify();
// cout << "subAXP: RHS now "; pRHS->print(); cout << std::endl;
        }
        
        // If this is a definition of the current register, then
        // update the map accordingly otherwise replace any use of
        // the register in the LHS of this assignment
        if (*pLHS == *it->first) {
            *it->second = *pRHS;
            isDef = true;
        }
        else {
            if(pLHS = pLHS->searchReplaceAll(it->first, it->second, change),
              change) {
                pLHS->simplify();
// cout << "subAXP: LHS now "; pLHS->print(); cout << std::endl;
            }
        }
    }
    return isDef;
}

/*==============================================================================
 * FUNCTION:        RTL::subAXP
 * OVERVIEW:        (See comment for Proc::subAXP)
 * PARAMETERS:      subMap -
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::subAXP(std::map<Exp*,Exp*>& subMap) {
    std::list<Exp*>::iterator it = expList.begin();
    while (it != expList.end()) {
// if (*it) {cerr << "\nAXP substitutution on `";
// (*it)->print(cerr);cerr<<"'\n";}
        if (expSubAXP((*it), (subMap))) {
            // This Exp was a definition of a substituted register and
            // so we must delete it here
            delete *it;
            it = expList.erase(it);
        } else {
            it++;
        }
    }
}

/*==============================================================================
 * FUNCTION:        RTL::searchReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - ptr to an expression to search for
 *                  replace - ptr to the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void RTL::searchAndReplace(Exp* search, Exp* replace)
{
    for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++)
    {
        Exp* pSrc = *it;
		bool ch;
        pSrc = pSrc->searchReplaceAll(search, replace, ch);
        // If the top level changed, must update the list
        if (pSrc != *it) {
            *it = pSrc;
        }
    }
}

/*==============================================================================
 * FUNCTION:        RTL::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool RTL::searchAll(Exp* search, std::list<Exp *> &result)
{
    bool found = false;
    for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++)
    {
        if ((*it)->searchAll(search, result)) {
            found = true;
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
    expList.clear();
}

#if 0
/*==============================================================================
 * FUNCTION:        searchExprForUses
 * OVERVIEW:        Search the expression for uses according to a given filter
 * PARAMETERS:      exp: expression to search
 *                  locMap - a map between locations and integer bit numbers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 * RETURNS:         Nothing, but reference sets added to
 *============================================================================*/
void searchExprForUses(Exp* exp, LocationMap& locMap, LocationFilter* filter,
                       BITSET& defSet, BITSET& useSet, BITSET& useUndefSet)
{

    OPER op = exp->getOp();
    int numVar = theSemTable[idx].iNumVarArgs;
    // Only interested in r[] or m[]
    if ((idx == opRegOf) || (idx == opMemOf)) {
        // This is the "bottom of the tree"; filter the whole expression
        if (filter->matches(*exp)) {
            int bit = locMap.toBit(*exp);

            // Record the use
            useSet.set(bit);

            // Add this to the use-before-definition set if necessary
            if (!defSet.test(bit)) {
                useUndefSet.set(bit);
            }
        }
    }

    // We have to recurse even into memofs, because they may contain some
    // register of expressions are used
    for (int i=0; i < numVar; i++) {
        // Recurse into the ith subexpression
        Exp* sub = exp->getSubExpr(i);
        searchExprForUses(sub, locMap, filter, defSet, useSet, useUndefSet);
        delete sub;
    }
}
#endif

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
 *                  size: size of the transfer, or -1 to be the same as the
 *                    first assign this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void RTL::insertAssign(Exp* pLhs, Exp* pRhs, bool prep,
                        int size /*= -1*/) {
    if (size == -1)
        size = 32;      // Ugh

    // Generate the assignment expression
    Exp* asgn = new AssignExp(size, pLhs, pRhs);
    if (prep)
        prependExp(asgn);
    else
        appendExp(asgn);
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
 *                  If size == -1, assumes there is already at least one assign-
 *                    ment in this RTL
 * NOTE:            Hopefully this is only a temporary measure
 * PARAMETERS:      pLhs: ptr to Exp to place on LHS
 *                  pRhs: ptr to Exp to place on the RHS
 *                  size: size of the transfer, or -1 to be the same as the
 *                    first assign this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void RTL::insertAfterTemps(Exp* pLhs, Exp* pRhs, int size /* = -1 */) {
    std::list<Exp*>::iterator it;
    // First skip all assignments with temps on LHS
    for (it = expList.begin(); it != expList.end(); it++) {
        if (!(*it)->isAssign())
            break;
        Exp* LHS = (*it)->getSubExp1();
        if (LHS->isTemp())
            break;
    }

    // Now check if the next Exp is an assignment
    if ((it == expList.end()) || (!(*it)->isAssign())) {
        // There isn't an assignment following. Use the previous Exp to insert
        // before
        if (it != expList.begin())
            it--;
    }

    if (size == -1)
        size = getSize();

    // Generate the assignment expression
    AssignExp* asgn = new AssignExp(32, pLhs, pRhs);

    // Insert before "it"
    expList.insert(it, asgn);
}

/*==============================================================================
 * FUNCTION:        RTL::getSize
 * OVERVIEW:        Get the "size" for this RTL. Just gets the size in bits of
 *                    the first assignment Exp
 * NOTE:            The size of the first assign may not be the size that you
 *                    want!
 * PARAMETERS:      None
 * RETURNS:         The size
 *============================================================================*/
int RTL::getSize() {
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++) {
        if ((*it)->isAssign())
            return ((AssignExp*)(*it))->getSize();
    }
    return 32;              // Default to 32 bits if no assignments
}

/*==============================================================================
 * FUNCTION:        RTL::forwardSubs
 * OVERVIEW:        Perform forward substitutions of temporaries (but not
 *                    tempNul) if possible. Useful where conditional assignments
 *                    are heavily used, and the simplification from forward
 *                    substitution is needed for analysis to work (e.g. pa-risc)
 * ASSUMPTION:      It is assumed that temporaries (other than tmpNul, which is
 *                    a sort of global) are only used within the current RTL
 * PARAMETERS:      None
 * RETURNS:         Nothing
 *============================================================================*/
void RTL::forwardSubs()
{
    std::map<Exp*, Exp*> temps;   // A map from left hand sides to right hand
                             // sides, suitable for substition
    std::map<Exp*, Exp*>::iterator mm;
    Exp* result;             // Result of a search
    std::list<Exp*> results;      // Another dummy for results
    Exp* srch;
    bool change;

    // Find the temporaries on the LHS, and make substitutions on
    // the right where they appear
    // Be careful with changes to the temps, or to components thereof
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++) {
        if ((*it)->isAssign()) continue;
        Exp* lhs = (*it)->getSubExp1();
        Exp* rhs = (*it)->getSubExp2();
        // Substitute the RHS, and LHS if in m[] etc
        for (mm = temps.begin(); mm != temps.end(); mm++) {
            if (mm->second == 0)        // See below
                // This temp assignment has been disabled by setting the ptr
                // to zero (see below)
                continue;
            srch = mm->first;
            rhs = rhs->searchReplaceAll(srch, mm->second, change);
            if (!(*lhs == *srch))
                lhs = lhs->searchReplaceAll(srch, mm->second, change);
        }
        if (
          ((*it)->getGuard() == 0) &&     // Must not be guarded!
          lhs->isTemp()) {
            // We have a temp. Add it to the map. (If it already exists,
            // then the mapping is updated rather than added)
            // The map has to be of Exp, not Exp*, for this to work.
            temps[lhs] = rhs;
        } else {
            // This is not assigning to a temp. Must check whether any temps
            // are now invalidated, for the purpose of substiution, by this
            // assignment
            for (mm = temps.begin(); mm != temps.end(); mm++) {
                if (mm->second->search(lhs, result)) {
                    // This temp is no longer usable for forward substitutions
                    // Rather than deleting it, we "set a flag" by clearing
                    // the map value (mm.second). This makes it ineligible for
                    // substitutions, but still available for deleting the
                    // assignment of. If we don't do this, then with
                    // tmp1 = r19 + r20;
                    // r19 = r19 + tmp1;
                    // we get the substitution, but the assignment to tmp1
                    // remains
                    mm->second = 0;
                }
            }
        }
    }

    // Now see if the assignments to the temps can go. Delete any entries from
    // the map for those that can't go
    for (mm = temps.begin(); mm != temps.end(); mm++) {
        for (it = expList.begin(); it != expList.end(); it++) {
            if ((*it)->isAssign()) {
                Exp* rhs = (*it)->getSubExp2();
                if (rhs->search(mm->first, result)) {
                    // Note: map::erase seems to return a void! So can't do the
                    // usual and safe mm = temps.erase(mm)
                    temps.erase(mm);
                    break;
                }
                // Temps can also appear on the LHS, e.g. as m[r[tmp]], but
                // ignore direct assignment to it (i.e. *lhs == *mm->first)
                Exp* lhs = (*it)->getSubExp2();
                if ((!(*lhs == *mm->first)) &&
                  (lhs->search(mm->first, result))) {
                    temps.erase(mm);
                    break;
                }
            } else if ((*it)->isFlagCall()) {
                // If used in a flag call, still used
                // Only difference with assignments is don't need to check "LHS"
                Exp* params = (*it)->getSubExp2();
                if (params->search(mm->first, result)) {
                    temps.erase(mm);
                    break;
                }
            }
        }
    }

    // Any entries left in the map can have their assignments deleted
    for (mm = temps.begin(); mm != temps.end(); mm++) {
        for (it = expList.begin(); it != expList.end(); ) {
            if (!(*it)->isAssign()) {it++; continue;}
            Exp* lhs = (*it)->getSubExp1();
            if (*lhs == *mm->first) {
                // Delete the assignment
                it = expList.erase(it);
                continue;
            }
            it++;
        }
    }

}

/*==============================================================================
 * FUNCTION:      RTL::areFlagsAffected
 * OVERVIEW:      Return true if this RTL affects the condition codes
 * NOTE:          Assumes that if there is a flag call Exp, then it is the last
 * PARAMETERS:    None
 * RETURNS:       Boolean as above
 *============================================================================*/
bool RTL::areFlagsAffected()
{
	if (expList.size() == 0) return false;
	// Get an iterator to the last RT
	std::list<Exp*>::iterator it = expList.end();
    if (it == expList.begin())
        return false;           // Not expressions at all
    it--;                       // Will now point to the end of the list
	// If it is a flag call, then the CCs are affected
	return (*it)->isFlagCall();
}


// serialize this rtl
bool RTL::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, (char)kind, false);
	saveValue(ouf, nativeAddr, false);

	saveFID(ouf, FID_RTL_NUMNATIVEBYTES);
	saveValue(ouf, numNativeBytes);

	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		saveFID(ouf, FID_RTL_EXP);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		assert((*it)->serialize(ouf, len));

		std::streampos now = ouf.tellp();
		assert((int)(now - posa) == len);
		ouf.seekp(pos);
		saveLen(ouf, len, true);
		ouf.seekp(now);
	}

	serialize_rest(ouf);

	saveFID(ouf, FID_RTL_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool RTL::serialize_rest(std::ostream &ouf)
{
	return true;
}

// deserialize an rtl
RTL *RTL::deserialize(std::istream &inf)
{
	RTL *rtl = NULL;
	ADDRESS a;
	char ch;
	loadValue(inf, ch, false);
	loadValue(inf, a, false);
	switch(ch) {
		case HL_NONE:
			rtl = new RTL(a);
			break;
		case CALL_RTL:
			rtl = new HLCall(a);
			break;
		case RET_RTL:
			rtl = new HLReturn(a);
			break;
		case JCOND_RTL:
			rtl = new HLJcond(a);
			break;
		case JUMP_RTL:
			rtl = new HLJump(a);
			break;
		case SCOND_RTL:
			rtl = new HLScond(a);
			break;
		case NWAYJUMP_RTL:
			rtl = new HLNwayJump(a);
			break;
		default:
			std::cerr << "WARNING: unknown rtl type!  ignoring, data will be lost!" << std::endl;
	}
	if (rtl) {
		int fid;

		while ((fid = loadFID(inf)) != -1 && fid != FID_RTL_END)
			rtl->deserialize_fid(inf, fid);
		assert(loadLen(inf) == 0);
	}

	return rtl;
}

bool RTL::deserialize_fid(std::istream &inf, int fid)
{
	switch (fid) {
		case FID_RTL_NUMNATIVEBYTES:
			loadValue(inf, numNativeBytes);
			break;
		case FID_RTL_EXP:
			{
				int len = loadLen(inf);
				std::streampos pos = inf.tellg();
				Exp *exp = Exp::deserialize(inf);
				if (exp) {
					assert((int)(inf.tellg() - pos) == len);
					expList.push_back(exp);
				} else {
					// unknown exp type, skip it
					inf.seekg(pos + (std::streamoff)len);
				}
			}
			break;
		default:
			skipFID(inf, fid);
			return false;
	}

	return true;
}

void RTL::generateCode(HLLCode &hll, BasicBlock *pbb)
{
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		if ((*it)->isAssign()) {
			hll.AddAssignmentStatement(pbb, *it);
		}
	}
}

bool RTL::getSSADefs(DefSet &defs, bool ssa)
{
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		if (!(*it)->isAssign()) continue;
		Exp *left = (*it)->getSubExp1();
		if (left->getOper() == opMemOf)
			continue;
		// if ssa still ok check if defs contains left
		if (ssa) {
			// bool found = false;
			for (DefSet::iterator it = defs.begin(); it != defs.end(); it++)
				if (*(*it).getLeft() == *left) {
					ssa = false;
					break;
				}
		}		
		// defs.insert(Def(this, it));  // Gcc won't have this
        // Presumably it no longer allows making a ref to a temporary
        Def dummy(this, it);
		defs.insert(dummy);
	}
	return ssa;
}

void RTL::SSAsubscript(SSACounts &counts)
{
	// for each expression in this RTL
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		// make sure it's an assign
		if (!(*it)->isAssign()) continue;
		// get the left
		Exp *left = (*it)->getSubExp1();

		UseSet u;
		// if it's a phi then we're only interested in uses on the left if it's a memof
		if ((*it)->getSubExp2()->getOper() == opPhi) {
			if (left->getOper() == opMemOf) {
				left->getUses(u, left);
				u.remove(left); // dont want the memOf
			}
		} else {
			// get the uses in this expression
			(*it)->getUses(u, *it);
		}

		// for each use
		for (UseSet::iterator uit = u.begin(); uit != u.end(); uit++) {
			(*uit).subscript(counts);
		}		

		if (left->getOper() == opMemOf)
			continue;

		// get the value of the left (if subscripted)
		Exp *leftval = left;
		if (leftval->getOper() == opSubscript) leftval = leftval->getSubExp1();

		// increase counts for the left
		counts.incSubscriptFor(leftval);
		// get subscript for left
		int ncount = counts.getMaxSubscriptFor(leftval);

		if (left->getOper() == opSubscript) {
			assert(left->getSubExp2()->getOper() == opIntConst);
			int ocount = ((Const*)left->getSubExp2())->getInt();
			if (ncount != ocount) {
				// update the count
				((Const*)left->getSubExp2())->setInt(ncount);
			}
		} else {
			// subscript the left
			(*it)->setSubExp1(new Binary(opSubscript, left->clone(), new Const(ncount)));
		}
	}
}


bool RTL::isUsedInPhi(Exp *e)
{
	// for each expression in this RTL
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		// make sure it's an assign
		if (!(*it)->isAssign()) continue;
		// get the right
		Exp *right = (*it)->getSubExp2();
		if (right->getOper() != opPhi)
			continue;

		for (Exp *l = right->getSubExp1(); l->getOper() != opNil; l = l->getSubExp2())
			if (*l->getSubExp1() == *e)
				return true;
	}
	return false;
}

void RTL::getUsesAfterDef(Exp *def, UseSet &uses)
{
	assert(kind == HL_NONE);

	std::list<Exp*>::iterator it;
	for (it = expList.begin(); it != expList.end(); it++) {
		if (*it == def)
			break;
	}

	for (it++; it != expList.end(); it++) {
		(*it)->getUsesOf(uses, *it, def->getSubExp1()->getSubExp1());
	}
}

bool RTL::containsDef(Exp *def)
{
	if (kind != HL_NONE) return false;

	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		if (*it == def)
			return true;
	}
	return false;
}

void RTL::getDefs(DefSet &defs, Exp *before_use)
{
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		if (before_use) {
			UseSet uses;
			Exp *e;
			Use u(e);
			(*it)->getUses(uses, *it);
			if (uses.find(before_use, u))
				break;
		}
		if ((*it)->isAssign()) {
			Def d(this, it);
			defs.insert(d);
		}
	}
}

void RTL::getUses(UseSet &uses, bool defIsUse)
{
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		(*it)->getUses(uses, *it, defIsUse);
	}
}

void RTL::getUsesOf(UseSet &uses, Exp *e)
{
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		(*it)->getUsesOf(uses, *it, e);
	}
}

void RTL::simplify()
{
	for (std::list<Exp*>::iterator it = expList.begin(); it != expList.end(); it++) {
		// simplify arithmetic of assignment
		Exp *e = (*it);
		if (!e->isAssign()) continue;
		Exp *e1 = e->getSubExp1()->simplifyArith()->clone();
		Exp *e2 = e->getSubExp2()->simplifyArith()->clone();
		e->setSubExp1(e1);
		e->setSubExp2(e2);
		// simplify the resultant expression
		*it = e->simplify();		
	}
}
