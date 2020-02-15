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

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"

#include <QTextStreamManipulator>


Assignment::Assignment(StmtType kind, SharedExp lhs)
    : TypingStatement(kind, VoidType::get())
    , m_lhs(lhs)
{
    if (lhs && lhs->isRegOfConst()) {
        UserProc *proc = lhs->access<Location>()->getProc();
        if (proc) {
            const RegNum n   = lhs->access<Const, 1>()->getInt();
            const Prog *prog = proc->getProg();

            m_type = SizeType::get(prog->getRegSizeByNum(n));
        }
    }
}


Assignment::Assignment(StmtType kind, SharedType ty, SharedExp lhs)
    : TypingStatement(kind, ty)
    , m_lhs(lhs)
{
}


Assignment::Assignment(const Assignment &other)
    : TypingStatement(other)
    , m_lhs(other.m_lhs ? other.m_lhs->clone() : nullptr)
{
}


Assignment::~Assignment()
{
}


Assignment &Assignment::operator=(const Assignment &other)
{
    TypingStatement::operator=(other);

    m_lhs = other.m_lhs ? other.m_lhs->clone() : nullptr;

    return *this;
}


bool Assignment::operator<(const Assignment &o)
{
    return *m_lhs < *o.m_lhs;
}


SharedConstType Assignment::getTypeForExp(SharedConstExp) const
{
    // assert(*lhs == *e); // No: local vs base expression
    return m_type;
}


SharedType Assignment::getTypeForExp(SharedExp /*e*/)
{
    // assert(*lhs == *e); // No: local vs base expression
    return m_type;
}


void Assignment::setTypeForExp(SharedExp /*e*/, SharedType ty)
{
    m_type = ty;
}


bool Assignment::definesLoc(SharedExp loc) const
{
    // For foo@[x:y], match of foo==loc OR whole thing == loc
    if (m_lhs->getOper() == opAt && *m_lhs->getSubExp1() == *loc) {
        return true;
    }

    return *m_lhs == *loc;
}


void Assignment::getDefinitions(LocationSet &defs, bool) const
{
    if (m_lhs->getOper() == opAt) { // foo@[m:n] really only defines foo
        defs.insert(m_lhs->getSubExp1());
    }
    else {
        defs.insert(m_lhs);
    }

    // Special case: flag calls define %CF (and others)
    if (m_lhs->isFlags()) {
        defs.insert(Terminal::get(opCF));
        defs.insert(Terminal::get(opZF));
        defs.insert(Terminal::get(opOF));
        defs.insert(Terminal::get(opNF));
    }
}


void Assignment::print(OStream &os) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";
    printCompact(os);
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
