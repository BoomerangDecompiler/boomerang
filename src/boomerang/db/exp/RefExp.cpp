#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RefExp.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/Statement.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/visitor/ExpModifier.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/type/type/IntegerType.h"


RefExp::RefExp(SharedExp e, Statement *d)
    : Unary(opSubscript, e)
    , m_def(d)
{
    assert(e);
}


std::shared_ptr<RefExp> RefExp::get(SharedExp e, Statement *def)
{
    return std::make_shared<RefExp>(e, def);
}


SharedExp RefExp::clone() const
{
    return RefExp::get(subExp1->clone(), m_def);
}


bool RefExp::operator==(const Exp& o) const
{
    if (o.getOper() == opWild) {
        return true;
    }

    if (o.getOper() != opSubscript) {
        return false;
    }

    if (!(*subExp1 == *o.getSubExp1())) {
        return false;
    }

    // Allow a def of (Statement*)-1 as a wild card
    if (m_def == (Statement *)-1) {
        return true;
    }

    assert(dynamic_cast<const RefExp *>(&o) != nullptr);

    // Allow a def of nullptr to match a def of an implicit assignment
    if (((RefExp&)o).m_def == (Statement *)-1) {
        return true;
    }

    if ((m_def == nullptr) && ((RefExp&)o).isImplicitDef()) {
        return true;
    }

    if ((((RefExp&)o).m_def == nullptr) && m_def && m_def->isImplicit()) {
        return true;
    }

    return m_def == ((RefExp&)o).m_def;
}


bool RefExp::operator<(const Exp& o) const
{
    if (opSubscript < o.getOper()) {
        return true;
    }

    if (opSubscript > o.getOper()) {
        return false;
    }

    if (*subExp1 < *((Unary&)o).getSubExp1()) {
        return true;
    }

    if (*((Unary&)o).getSubExp1() < *subExp1) {
        return false;
    }

    // Allow a wildcard def to match any
    if (m_def == (Statement *)-1) {
        return false; // Not less (equal)
    }

    if (((RefExp&)o).m_def == (Statement *)-1) {
        return false;
    }

    return m_def < ((RefExp&)o).m_def;
}


bool RefExp::operator*=(const Exp& o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return *subExp1 *= *other;
}


bool RefExp::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
    if (Exp::match(pattern, bindings)) {
        return true;
    }

#ifdef DEBUG_MATCH
    LOG_MSG("Matching %1 to %2.", this, pattern);
#endif

    if (pattern.endsWith('}')) {
        if ((pattern[pattern.size() - 2] == '-') && (m_def == nullptr)) {
            return subExp1->match(pattern.left(pattern.size() - 3), bindings); // remove {-}
        }

        int end = pattern.lastIndexOf('{');

        if (end != -1) {
            // "prefix {number ...}" -> number matches first def ?
            if (pattern.midRef(end + 1).toInt() == m_def->getNumber()) {
                // match "prefix"
                return subExp1->match(pattern.left(end - 1), bindings);
            }
        }
    }

    return false;
}


SharedExp RefExp::polySimplify(bool& bMod)
{
    SharedExp res = shared_from_this();

    SharedExp tmp = subExp1->polySimplify(bMod);

    if (bMod) {
        subExp1 = tmp;
        return res;
    }

    /*
     * This is a nasty hack.  We assume that %DF{0} is 0.  This happens when string instructions are used without first
     * clearing the direction flag.  By convention, the direction flag is assumed to be clear on entry to a
     * procedure.
     */
    if ((subExp1->getOper() == opDF) && (m_def == nullptr)) {
        res  = Const::get(int(0));
        bMod = true;
        return res;
    }

    // another hack, this time for aliasing
    // FIXME: do we really want this now? Pentium specific, and only handles ax/eax (not al or ah)
    if (subExp1->isRegN(0) &&                                                     // r0 (ax)
        m_def && m_def->isAssign() && ((Assign *)m_def)->getLeft()->isRegN(24)) { // r24 (eax)
        res  = std::make_shared<TypedExp>(IntegerType::get(16), RefExp::get(Location::regOf(24), m_def));
        bMod = true;
        return res;
    }

    // Was code here for bypassing phi statements that are now redundant

    return res;
}


bool RefExp::accept(ExpVisitor *v)
{
    bool visitChildren = false;
    bool ret = v->visit(shared_from_base<RefExp>(), visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret) {
        ret = subExp1->accept(v);
    }

    return ret;
}


SharedExp RefExp::accept(ExpModifier *v)
{
    bool visitChildren = true;
    auto ret     = v->preVisit(shared_from_base<RefExp>(), visitChildren);
    auto ref_ret = std::dynamic_pointer_cast<RefExp>(ret);

    if (visitChildren) {
        subExp1 = subExp1->accept(v);
    }

    // TODO: handle the case where Exp modifier changed type of Exp, currently just not calling postVisit!
    if (ref_ret) {
        return v->postVisit(ref_ret);
    }

    return ret;
}


void RefExp::printx(int ind) const
{
    LOG_VERBOSE("%1%2", QString(ind, ' '), operToString(m_oper));
    LOG_VERBOSE("{");

    if (m_def == nullptr) {
        LOG_VERBOSE("nullptr");
    }
    else {
        LOG_VERBOSE("%1=%2", HostAddress(m_def).toString(), m_def->getNumber());
    }

    LOG_VERBOSE("}");
    printChild(subExp1, ind);
}


bool RefExp::isImplicitDef() const
{
    return m_def == nullptr || m_def->getKind() == STMT_IMPASSIGN;
}


void RefExp::print(QTextStream& os, bool html) const
{
    if (subExp1) {
        subExp1->print(os, html);
    }
    else {
        os << "<nullptr>";
    }

    if (html) {
        os << "<sub>";
    }
    else {
        os << "{";
    }

    if (m_def == (Statement *)-1) {
        os << "WILD";
    }
    else if (m_def) {
        if (html) {
            os << "<a href=\"#stmt" << m_def->getNumber() << "\">";
        }

        m_def->printNum(os);

        if (html) {
            os << "</a>";
        }
    }
    else {
        os << "-"; // So you can tell the difference with {0}
    }

    if (html) {
        os << "</sub>";
    }
    else {
        os << "}";
    }
}


SharedExp RefExp::genConstraints(SharedExp result)
{
    OPER subOp = subExp1->getOper();

    switch (subOp)
    {
    case opRegOf:
    case opParam:
    case opGlobal:
    case opLocal:
        return Binary::get(opEquals, Unary::get(opTypeOf, this->clone()), result->clone());

    default:
        break;
    }

    return Terminal::get(opTrue);
}

SharedExp RefExp::addSubscript(Statement* _def)
{
    m_def = _def;
    return shared_from_this();
}


void RefExp::setDef(Statement* _def)
{
//         assert(_def != nullptr);
    m_def = _def;
}

