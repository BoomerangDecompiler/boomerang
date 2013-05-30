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
 * \file       rtl.cpp
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
 * \param   listStmt - ptr to existing list of Statement
 *
 ******************************************************************************/
RTL::RTL(ADDRESS instNativeAddr, const std::list<Statement*>* listStmt /*= nullptr*/)
    : nativeAddr(instNativeAddr) {
    if (listStmt)
        *(std::list<Statement*>*)this = *listStmt;
}

/***************************************************************************//**
 * \brief        Copy constructor. A deep clone is made of the given object
 *                    so that the lists of Exps do not share memory.
 * \param        other RTL to copy from
 ******************************************************************************/
RTL::RTL(const RTL& other) : nativeAddr(other.nativeAddr) {
    for (auto it = other.begin(); it != other.end(); it++) {
        push_back((*it)->clone());
    }
}

/***************************************************************************//**
 * \brief        Assignment copy (deep).
 * \param        other - RTL to copy
 * \returns             a reference to this object
 ******************************************************************************/
RTL& RTL::operator=(const RTL& other) {
    if (this != &other) {
        // Do a deep copy always
        clear();
        const_iterator it;
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
RTL* RTL::clone() const {
    std::list<Statement*> le;
    for (const_iterator it = begin(); it != end(); it++) {
        le.push_back((*it)->clone());
    }
    return new RTL(nativeAddr, &le);
}

/***************************************************************************//**
 * \brief        Make a copy of this RTLs list of Exp* to the given list
 * \param        dest Ref to empty list to copy to
 ******************************************************************************/
void RTL::deepCopyList(std::list<Statement*>& dest) const {
    for (Statement * it : *this) {
        dest.push_back(it->clone());
    }
}

/***************************************************************************//**
 * \brief        Append the given Statement at the end of this RTL
 * \note            Exception: Leaves any flag call at the end (so may push exp
 *                     to second last position, instead of last)
 * \note            stmt is NOT copied. This is different to how UQBT was!
 * \param        s pointer to Statement to append
 ******************************************************************************/
void RTL::appendStmt(Statement* s) {
    assert(s!=nullptr);
    if (not empty()) {
        if (back()->isFlagAssgn()) {
            iterator it = end();
            insert(--it, s);
            return;
        }
    }
    push_back(s);
}

/***************************************************************************//**
 * \brief        Append a given list of Statements to this RTL
 * \note            A copy of the Statements in le are appended
 * \param        rtl list of Statements to insert
 ******************************************************************************/
void RTL::appendListStmt(std::list<Statement*>& le) {
    for (Statement * it : le) {
        push_back(it->clone());
    }
}

/***************************************************************************//**
 * \brief   Prints this object to a stream in text form.
 * \param   os - stream to output to (often cout or cerr)
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
        if (stmt)
            stmt->print(os, html);
        // Note: we only put newlines where needed. So none at the end of
        // Statement::print; one here to separate from other statements
        if (html)
            os << "</td></tr>";
        os << "\n";
        bFirst = false;
    }
    if (empty())
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
 * \brief   Output operator for RTL*
 *          Just makes it easier to use e.g. std::cerr << myRTLptr
 * \param   os output stream to send to
 * \param   r ptr to RTL to print to the stream
 * \returns os (for concatenation)
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const RTL* r) {
    if (r == nullptr) {
        os << "nullptr ";
        return os;
    }
    r->print(os);
    return os;
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
    if (this->empty())
        return false;
    Statement* last = this->back();
    return last->getKind() == STMT_CALL;
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
