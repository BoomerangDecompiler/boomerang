#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Assignment.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/util/Log.h"


Assignment::Assignment(SharedExp lhs)
    : TypingStatement(VoidType::get())
    , m_lhs(lhs)
{
    if (lhs && lhs->isRegOf()) {
        int n = lhs->access<Const, 1>()->getInt();

        if (lhs->access<Location>()->getProc()) {
            m_type = SizeType::get(lhs->access<Location>()->getProc()->getProg()->getRegSize(n));
        }
    }
}


Assignment::Assignment(SharedType ty, SharedExp lhs)
    : TypingStatement(ty)
    , m_lhs(lhs)
{
}


Assignment::~Assignment()
{
}


SharedConstType Assignment::getTypeFor(SharedConstExp) const
{
    // assert(*lhs == *e); // No: local vs base expression
    return m_type;
}


SharedType Assignment::getTypeFor(SharedExp /*e*/)
{
    // assert(*lhs == *e); // No: local vs base expression
    return m_type;
}


void Assignment::setTypeFor(SharedExp /*e*/, SharedType ty)
{
    // assert(*lhs == *e);
    SharedType oldType = m_type;

    m_type = ty;

    if (DEBUG_TA && (oldType != ty)) {
        LOG_VERBOSE("    Changed type of %1 (type was %2)", this, oldType->getCtype());
    }
}



bool Assignment::definesLoc(SharedExp loc) const
{
    if (m_lhs->getOper() == opAt) {     // For foo@[x:y], match of foo==loc OR whole thing == loc
        if (*m_lhs->getSubExp1() == *loc) {
            return true;
        }
    }

    return *m_lhs == *loc;
}


bool Assignment::usesExp(const Exp& e) const
{
    SharedExp where = nullptr;

    return (m_lhs->isMemOf() || m_lhs->isRegOf()) && m_lhs->getSubExp1()->search(e, where);
}


void Assignment::getDefinitions(LocationSet& defs) const
{
    if (m_lhs->getOper() == opAt) {     // foo@[m:n] really only defines foo
        defs.insert(m_lhs->getSubExp1());
    }
    else {
        defs.insert(m_lhs);
    }

    // Special case: flag calls define %CF (and others)
    if (m_lhs->isFlags()) {
        defs.insert(Terminal::get(opCF));
        defs.insert(Terminal::get(opZF));
    }
}


void Assignment::print(QTextStream& os, bool html) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

    if (html) {
        os << "</td><td>";
        os << "<a name=\"stmt" << m_number << "\">";
    }

    printCompact(os, html);

    if (html) {
        os << "</a>";
    }
}


void Assignment::simplifyAddr()
{
    m_lhs = m_lhs->simplifyAddr();
}


SharedExp Assignment::getLeft() const
{
    return m_lhs;
}


void Assignment::setLeft(SharedExp e)
{
    m_lhs = e;
}
