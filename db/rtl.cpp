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

/***************************************************************************//**
 * \file       rtl.cc
 * \brief   Implementation of the classes that describe a low level RTL (
 *               register transfer list)
 ******************************************************************************/

/*
 * $Revision$    // 1.33.2.3
 *
 * 08 Apr 02 - Mike: Changes for boomerang
 * 13 May 02 - Mike: expList is no longer a pointer
 * 15 May 02 - Mike: Fixed a nasty bug in updateExp (when update with same
 *                expression as existing)
 * 25 Jul 02 - Mike: RTL is now list of Statements
 */

#include <cassert>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <iomanip>            // For setfill
#include <sstream>
#include <cstring>
#include "types.h"
#include "statement.h"
#include "exp.h"
#include "type.h"
#include "register.h"
#include "cfg.h"
#include "proc.h"            // For printing proc names
#include "rtl.h"
#include "prog.h"
#include "hllcode.h"
#include "util.h"
#include "boomerang.h"
#include "visitor.h"
#include "log.h"
// For some reason, MSVC 5.00 complains about use of undefined types a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"        // For MSVC 5.00
#endif

/******************************************************************************
 * RTL methods.
 * Class RTL represents low-level register transfer lists.
 *****************************************************************************/

RTL::RTL()
  : nativeAddr(ADDRESS::g(0L))
{ }

/***************************************************************************//**
 * \brief   Constructor.
 * \param   instNativeAddr - the native address of the instruction
 * \param   listExp - ptr to existing list of Exps
 * \returns N/a
 ******************************************************************************/
RTL::RTL(ADDRESS instNativeAddr, std::list<Statement*>* listStmt /*= nullptr*/)
    : nativeAddr(instNativeAddr) {
    if (listStmt)
        *(std::list<Statement*>*)this = *listStmt;
}

/***************************************************************************//**
 * \brief        Copy constructor. A deep clone is made of the given object
 *                    so that the lists of Exps do not share memory.
 * \param        other: RTL to copy from
 ******************************************************************************/
RTL::RTL(const RTL& other) : nativeAddr(other.nativeAddr) {
    std::list<Statement*>::const_iterator it;
    for (it = other.begin(); it != other.end(); it++) {
        push_back((*it)->clone());
    }
}

RTL::~RTL() { }

/***************************************************************************//**
 * \brief        Assignment copy (deep).
 * \param        other - RTL to copy
 * \returns             a reference to this object
 ******************************************************************************/
RTL& RTL::operator=(RTL& other) {
    if (this != &other) {
        // Do a deep copy always
        iterator it;
        for (it = other.begin(); it != other.end(); it++)
            push_back((*it)->clone());

        nativeAddr = other.nativeAddr;
    }
    return *this;
}

// Return a deep copy, including a deep copy of the list of Statements
/***************************************************************************//**
 * \brief        Deep copy clone; deleting the clone will not affect this
 *                     RTL object
 * \returns             Pointer to a new RTL that is a clone of this one
 ******************************************************************************/
RTL* RTL::clone() {
    std::list<Statement*> le;
    for (iterator it = begin(); it != end(); it++) {
        le.push_back((*it)->clone());
    }
    return new RTL(nativeAddr, &le);
}

// visit this RTL, and all its Statements
bool RTL::accept(StmtVisitor* visitor) {
    // Might want to do something at the RTL level:
    if (!visitor->visit(this)) return false;
    iterator it;
    for (it = begin(); it != end(); it++) {
        if (! (*it)->accept(visitor))
            return false;
    }
    return true;
}

/***************************************************************************//**
 * \brief        Make a copy of this RTLs list of Exp* to the given list
 * \param        Ref to empty list to copy to
 * \returns             Nothing
 ******************************************************************************/
void RTL::deepCopyList(std::list<Statement*>& dest) {
    std::list<Statement*>::iterator it;

    for (it = begin(); it != end(); it++) {
        dest.push_back((*it)->clone());
    }
}

/***************************************************************************//**
 * \brief        Append the given Statement at the end of this RTL
 * \note            Exception: Leaves any flag call at the end (so may push exp
 *                     to second last position, instead of last)
 * \note            stmt is NOT copied. This is different to how UQBT was!
 * \param        s: pointer to Statement to append
 * \returns             Nothing
 ******************************************************************************/
void RTL::appendStmt(Statement* s) {
    if (size()) {
        if (back()->isFlagAssgn()) {
            iterator it = end();
            insert(--it, s);
            return;
        }
    }
    push_back(s);
}

/***************************************************************************//**
 * \brief        Prepend the given Statement at the start of this RTL
 * \note            No clone of the statement is made. This is different to how UQBT was
 * \param        s: Ptr to Statement to prepend
 ******************************************************************************/
void RTL::prependStmt(Statement* s) {
    push_front(s);
}

/***************************************************************************//**
 * \brief        Append a given list of Statements to this RTL
 * \note            A copy of the Statements in le are appended
 * \param        rtl: list of Exps to insert
 ******************************************************************************/
void RTL::appendListStmt(std::list<Statement*>& le) {
    for (Statement * it : le) {
        push_back(it->clone());
    }
}

/***************************************************************************//**
 * \brief        Append the Statemens of another RTL to this object
 * \note            A copy of the Statements in r are appended
 * \param        r: reterence to RTL whose Exps we are to insert
 ******************************************************************************/
void RTL::appendRTL(RTL& r) {
    appendListStmt(r);
}

/***************************************************************************//**
 * \brief        Insert the given Statement before index i
 * \note            No copy of stmt is made. This is different to UQBT
 * \param        s: pointer to the Statement to insert
 *                    i: position to insert before (0 = first)
 ******************************************************************************/
void RTL::insertStmt(Statement* s, unsigned i) {
    // Check that position i is not out of bounds
    assert (i < size() || size() == 0);

    // Find the position
    iterator pp = begin();
    for (; i > 0; i--, pp++);

    // Do the insertion
    insert(pp, s);
}

void RTL::insertStmt(Statement* s, iterator it) {
    insert(it, s);
}

void RTL::deleteStmt(unsigned i) {
    // check that position i is not out of bounds
    assert (i < size());

    // find the position
    iterator pp = this->begin();
    std::advance(pp,i);
    // do the delete
    this->erase(pp);
}

/***************************************************************************//**
 * \brief        Get the number of Statements in this RTL
 * \returns             Integer number of Statements
 ******************************************************************************/
size_t RTL::getNumStmt() {
    return this->size();
}

/***************************************************************************//**
 * \brief   Provides indexing on a list. Changed from operator[] so that
 *          we keep in mind it is linear in its execution time.
 * \param   i - the index of the element we want (0 = first)
 * \returns the element at the given index or nullptr if the index is out
 *          of bounds
 ******************************************************************************/
Statement* RTL::elementAt(unsigned i) {
    iterator it=this->begin();
    if(i>=this->size())
        return nullptr;
    std::advance(it,i);
    return *it;
}

/***************************************************************************//**
 * \brief   Prints this object to a stream in text form.
 * \param   os - stream to output to (often cout or cerr)
 * \returns <nothing>
 ******************************************************************************/
void RTL::print(std::ostream& os /*= cout*/, bool html /*=false*/) const {

    if (html)
        os << "<tr><td>";
    // print out the instruction address of this RTL
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << std::dec << std::setfill(' ');      // Ugh - why is this needed?
    if (html)
        os << "</td>";

    // Print the statements
    // First line has 8 extra chars as above
    bool bFirst = true;
    for (Statement * stmt : *this) {
        if (html) {
            if (!bFirst) os << "<tr><td></td>";
            os << "<td width=\"50\" align=\"center\">";
        } else {
            if (bFirst) os << " ";
            else        os << std::setw(9) << " ";
        }
        if (stmt) stmt->print(os, html);
        // Note: we only put newlines where needed. So none at the end of
        // Statement::print; one here to separate from other statements
        if (html)
            os << "</td></tr>";
        os << "\n";
        bFirst = false;
    }
    if (this->empty())
        os << std::endl;       // New line for NOP
}

void RTL::dump() {
    print(std::cerr);
}

extern char debug_buffer[];

char* RTL::prints() const {
    std::ostringstream ost;
    print(ost);
    strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
    debug_buffer[DEBUG_BUFSIZE-1] = '\0';
    return debug_buffer;
}

/***************************************************************************//**
 * \brief        Output operator for RTL*
 * Just makes it easier to use e.g. std::cerr << myRTLptr
 * \param os: output stream to send to
 *        r: ptr to RTL to print to the stream
 * \returns os (for concatenation)
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, RTL* r) {
    if (r == nullptr) {
        os << "nullptr ";
        return os;
    }
    r->print(os);
    return os;
}
/***************************************************************************//**
 * \brief Set the RTL's source address nativeAddr field
 * \param addr Native address
 ******************************************************************************/
void RTL::updateAddress(ADDRESS addr) {
    nativeAddr = addr;
}

/***************************************************************************//**
 * \brief Replace all instances of search with replace.
 * \param search - ptr to an expression to search for
 * \param replace - ptr to the expression with which to replace it
 * \returns true if replacement took place
 ******************************************************************************/
bool RTL::searchAndReplace(Exp* search, Exp* replace) {
    bool ch = false;
    for (iterator it = this->begin(); it != this->end(); it++)
        ch |= (*it)->searchAndReplace(search, replace);
    return ch;
}
//
//

/***************************************************************************//**
 * \brief        Searches for all instances of "search" and adds them to "result"
 * in reverse nesting order. The search is optionally type sensitive.
 * \note out of date doc ?
 * \param search - a location to search for
 * \param result - a list which will have any matching exprs
 *                 appended to it
 * \returns true if there were any matches
 ******************************************************************************/
bool RTL::searchAll(Exp* search, std::list<Exp *> &result) {
    bool found = false;
    for (Statement * e : *this) {
        Exp* res;
        if (e->search(search, res)) {
            found = true;
            result.push_back(res);
        }
    }
    return found;
}

/***************************************************************************//**
 * \brief        Clear the list of Exps
 * \returns             Nothing
 ******************************************************************************/

/***************************************************************************//**
 * \brief       Get the "type" for this RTL. Just gets the type of
 *              the first assignment Exp
 * \note        The type of the first assign may not be the type that you
 *              want!
 * \returns     A pointer to the type
 ******************************************************************************/
Type* RTL::getType() {
    for (Statement *e : *this) {
        if (e->isAssign())
            return ((Assign*)e)->getType();
    }
    return new IntegerType();    //    Default to 32 bit integer if no assignments
}

/***************************************************************************//**
 * \brief      Return true if this RTL affects the condition codes
 * \note          Assumes that if there is a flag call Exp, then it is the last
 * \returns           Boolean as above
 ******************************************************************************/
bool RTL::areFlagsAffected() {
    if (this->empty())
        return false;
    Statement *e = this->back();
    // If it is a flag call, then the CCs are affected
    return e->isFlagAssgn();
}

// Generata code for all statements in this rtl
void RTL::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    for (Statement * st : *this) {
        st->generateCode(hll, pbb, indLevel);
    }
}

void RTL::simplify() {
    for (iterator it = this->begin(); it != this->end(); ) {
        Statement *s = *it;
        s->simplify();
        if (s->isBranch()) {
            Exp *cond =     ((BranchStatement*)s)->getCondExpr();
            if (cond && cond->getOper() == opIntConst) {
                if (((Const*)cond)->getInt() == 0) {
                    if (VERBOSE)
                        LOG << "removing branch with false condition at " << getAddress()  << " " << *it << "\n";
                    it = this->erase(it);
                    continue;
                } else {
                    if (VERBOSE)
                        LOG << "replacing branch with true condition with goto at " << getAddress() << " " << *it <<
                               "\n";
                    *it = new GotoStatement(((BranchStatement*)s)->getFixedDest());
                }
            }
        } else if (s->isAssign()) {
            Exp* guard = ((Assign*)s)->getGuard();
            if (guard && (guard->isFalse() || (guard->isIntConst() && ((Const*)guard)->getInt() == 0))) {
                // This assignment statement can be deleted
                if (VERBOSE)
                    LOG << "removing assignment with false guard at " << getAddress() << " " << *it << "\n";
                it = this->erase(it);
                continue;
            }
        }
        it++;
    }
}
// Is this RTL a compare instruction? If so, the passed register and compared value (a semantic string) are set.

bool RTL::isCall() {
    return isKindOf(STMT_CALL);
}

// Use this slow function when you can't be sure that the HL Statement is last
// Get the "special" (High Level) Statement this RTL (else nullptr)
Statement* RTL::getHlStmt() {
    reverse_iterator rit;
    for (rit = this->rbegin(); rit != this->rend(); rit++) {
        if ((*rit)->getKind() != STMT_ASSIGN)
            return *rit;
    }
    return nullptr;
}

//int RTL::setConscripts(int n, bool bClear) {
//    StmtConscriptSetter ssc(n, bClear);
//    accept(&ssc);
//    return ssc.getLast();
//}

bool RTL::isKindOf(STMT_KIND k) {
    if (this->empty())
        return false;
    Statement* last = this->back();
    return last->getKind() == k;
}
