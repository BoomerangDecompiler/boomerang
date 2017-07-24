#include "Assignment.h"

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/Proc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/TypeVal.h"
#include "boomerang/type/Type.h"

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


SharedType Assignment::getTypeFor(SharedExp /*e*/) const
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
        LOG << "    changed type of " << this << "  (type was " << oldType->getCtype() << ")\n";
    }
}


void Assignment::dfaTypeAnalysis(bool& ch)
{
    auto sig = m_proc->getSignature();

    // Don't do this for the common case of an ordinary local,
    // since it generates hundreds of implicit references,
    // without any new type information
    if (m_lhs->isMemOf() && !sig->isStackLocal(m_proc->getProg(), m_lhs)) {
        SharedExp addr = m_lhs->getSubExp1();
        // Meet the assignment type with *(type of the address)
        SharedType addrType = addr->ascendType();
        SharedType memofType;

        if (addrType->resolvesToPointer()) {
            memofType = addrType->as<PointerType>()->getPointsTo();
        }
        else {
            memofType = VoidType::get();
        }

        m_type = m_type->meetWith(memofType, ch);
        // Push down the fact that the memof operand is a pointer to the assignment type
        addrType = PointerType::get(m_type);
        addr->descendType(addrType, ch, this);
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


void Assignment::genConstraints(LocationSet& cons)
{
    // Almost every assignment has at least a size from decoding
    // MVE: do/will PhiAssign's have a valid type? Why not?
    if (m_type) {
        cons.insert(Binary::get(opEquals,
                                Unary::get(opTypeOf, RefExp::get(m_lhs, this)),
                                TypeVal::get(m_type)));
    }
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


SharedExp Assignment::getLeft()
{
    return m_lhs;
}


const SharedExp& Assignment::getLeft() const
{
    return m_lhs;
}


void Assignment::setLeft(SharedExp e)
{
    m_lhs = e;
}
