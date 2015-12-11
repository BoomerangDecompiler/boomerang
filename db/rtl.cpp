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

/***************************************************************************/ /**
  * \file       rtl.cpp
  * \brief   Implementation of the classes that describe a low level RTL (
  *               register transfer list)
  ******************************************************************************/


#include "rtl.h"

#include "boomerang.h"                  // for LOG_VERBOSE
#include "exp.h"                        // for Const, Exp, DEBUG_BUFSIZE
#include "log.h"                        // for Log
#include "operator.h"                   // for OPER::opIntConst
#include "statement.h"                  // for Instruction, etc
#include "types.h"                      // for ADDRESS

#include <QString>
#include <QTextStream>
#include <stdio.h>                      // for stderr
#include <cassert>                      // for assert
#include <cstring>                      // for strncpy

/******************************************************************************
 * RTL methods.
 * Class RTL represents low-level register transfer lists.
 *****************************************************************************/

RTL::RTL() : nativeAddr(ADDRESS::g(0L)) {}

/***************************************************************************/ /**
  * \brief   Constructor.
  * \param   instNativeAddr - the native address of the instruction
  * \param   listStmt - ptr to existing list of Statement
  *
  ******************************************************************************/
RTL::RTL(ADDRESS instNativeAddr, const std::list<Instruction *> *listStmt /*= nullptr*/) : nativeAddr(instNativeAddr) {
    if (listStmt)
        *(std::list<Instruction *> *)this = *listStmt;
}

/***************************************************************************/ /**
  * \brief        Copy constructor. A deep clone is made of the given object
  *                    so that the lists of Exps do not share memory.
  * \param        other RTL to copy from
  ******************************************************************************/
RTL::RTL(const RTL &other) : std::list<Instruction *>(), nativeAddr(other.nativeAddr) {
    for (auto const &elem : other) {
        push_back(elem->clone());
    }
}

RTL::~RTL()
{
    qDeleteAll(*this);
}

/***************************************************************************/ /**
  * \brief        Assignment copy (deep).
  * \param        other - RTL to copy
  * \returns             a reference to this object
  ******************************************************************************/
RTL &RTL::operator=(const RTL &other) {
    if (this != &other) {
        qDeleteAll(*this);
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
/***************************************************************************/ /**
  * \brief        Deep copy clone; deleting the clone will not affect this
  *                     RTL object
  * \returns             Pointer to a new RTL that is a clone of this one
  ******************************************************************************/
RTL *RTL::clone() const {
    std::list<Instruction *> le;
    for (auto const &elem : *this) {
        le.push_back((elem)->clone());
    }
    return new RTL(nativeAddr, &le);
}

/***************************************************************************/ /**
  * \brief        Make a copy of this RTLs list of Exp* to the given list
  * \param        dest Ref to empty list to copy to
  ******************************************************************************/
void RTL::deepCopyList(std::list<Instruction *> &dest) const {
    for (Instruction *it : *this) {
        dest.push_back(it->clone());
    }
}

/***************************************************************************/ /**
  * \brief        Append the given Statement at the end of this RTL
  * \note            Exception: Leaves any flag call at the end (so may push exp
  *                     to second last position, instead of last)
  * \note            stmt is NOT copied. This is different to how UQBT was!
  * \param        s pointer to Statement to append
  ******************************************************************************/
void RTL::appendStmt(Instruction *s) {
    assert(s != nullptr);
    if (not empty()) {
        if (back()->isFlagAssgn()) {
            iterator it = end();
            insert(--it, s);
            return;
        }
    }
    push_back(s);
}

/***************************************************************************/ /**
  * \brief  Append a given list of Statements to this RTL
  * \note   A copy of the Statements in le are appended
  * \param  le - list of Statements to insert
  ******************************************************************************/
void RTL::appendListStmt(std::list<Instruction *> &le) {
    for (Instruction *it : le) {
        push_back(it->clone());
    }
}

/***************************************************************************/ /**
  * \brief   Prints this object to a stream in text form.
  * \param   os - stream to output to (often cout or cerr)
  * \param   html - if true output is in html
  ******************************************************************************/
void RTL::print(QTextStream &os /*= cout*/, bool html /*=false*/) const {

    if (html)
        os << "<tr><td>";
    // print out the instruction address of this RTL
    os << QString("%1").arg(nativeAddr.m_value,8,16,QChar('0'));
    if (html)
        os << "</td>";

    // Print the statements
    // First line has 8 extra chars as above
    bool bFirst = true;
    for (Instruction *stmt : *this) {
        if (html) {
            if (!bFirst)
                os << "<tr><td></td>";
            os << "<td width=\"50\" align=\"center\">";
        } else {
            if (bFirst)
                os << " ";
            else
                os << qSetFieldWidth(9) << " " << qSetFieldWidth(0);
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
        os << '\n'; // New line for NOP
}

void RTL::dump() {
    QTextStream q_cerr(stderr);
    print(q_cerr);
}

extern char debug_buffer[];

char *RTL::prints() const {
    QString tgt;
    QTextStream ost(&tgt);
    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}

/***************************************************************************/ /**
  * \brief   Output operator for RTL*
  *          Just makes it easier to use e.g. LOG_STREAM() << myRTLptr
  * \param   os output stream to send to
  * \param   r ptr to RTL to print to the stream
  * \returns os (for concatenation)
  ******************************************************************************/
QTextStream &operator<<(QTextStream &os, const RTL *r) {
    if (r == nullptr) {
        os << "nullptr ";
        return os;
    }
    r->print(os);
    return os;
}

/***************************************************************************/ /**
  * \brief      Return true if this RTL affects the condition codes
  * \note          Assumes that if there is a flag call Exp, then it is the last
  * \returns           Boolean as above
  ******************************************************************************/
bool RTL::areFlagsAffected() {
    if (this->empty())
        return false;
    Instruction *e = this->back();
    // If it is a flag call, then the CCs are affected
    return e->isFlagAssgn();
}

void RTL::simplify() {
    for (iterator it = begin(); it != end();) {
        Instruction *s = *it;
        s->simplify();
        if (s->isBranch()) {
            Exp *cond = ((BranchStatement *)s)->getCondExpr();
            if (cond && cond->getOper() == opIntConst) {
                if (((Const *)cond)->getInt() == 0) {
                    LOG_VERBOSE(1) << "removing branch with false condition at " << getAddress() << " " << *it << "\n";
                    it = this->erase(it);
                    continue;
                }
                LOG_VERBOSE(1) << "replacing branch with true condition with goto at " << getAddress() << " " << *it
                               << "\n";
                *it = new GotoStatement(((BranchStatement *)s)->getFixedDest());
            }
        } else if (s->isAssign()) {
            Exp *guard = ((Assign *)s)->getGuard();
            if (guard && (guard->isFalse() || (guard->isIntConst() && ((Const *)guard)->getInt() == 0))) {
                // This assignment statement can be deleted
                LOG_VERBOSE(1) << "removing assignment with false guard at " << getAddress() << " " << *it << "\n";
                it = erase(it);
                continue;
            }
        }
        it++;
    }
}
// Is this RTL a compare instruction? If so, the passed register and compared value (a semantic string) are set.

bool RTL::isCall() {
    if (empty())
        return false;
    Instruction *last = this->back();
    return last->getKind() == STMT_CALL;
}

// Use this slow function when you can't be sure that the HL Statement is last
// Get the "special" (High Level) Statement this RTL (else nullptr)
Instruction *RTL::getHlStmt() {
    reverse_iterator rit;
    for (rit = this->rbegin(); rit != this->rend(); rit++) {
        if ((*rit)->getKind() != STMT_ASSIGN)
            return *rit;
    }
    return nullptr;
}
