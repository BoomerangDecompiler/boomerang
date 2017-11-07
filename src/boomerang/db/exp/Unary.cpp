#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Unary.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/ExpModifier.h"
#include "boomerang/util/Log.h"


Unary::Unary(OPER _op, SharedExp e)
    : Exp(_op)
    , subExp1(e)
{
    assert(subExp1);
}


Unary::Unary(const Unary& o)
    : Exp(o.m_oper)
{
    subExp1 = o.subExp1->clone();
    assert(subExp1);
}


Unary::~Unary()
{
    // Remember to ;//delete all children
    if (subExp1 != nullptr) {
        // delete subExp1;
    }
}


void Unary::setSubExp1(SharedExp e)
{
    subExp1 = e;
    assert(subExp1);
}


SharedExp Unary::getSubExp1()
{
    assert(subExp1);
    return subExp1;
}


SharedConstExp Unary::getSubExp1() const
{
    assert(subExp1);
    return subExp1;
}


SharedExp& Unary::refSubExp1()
{
    assert(subExp1);
    return subExp1;
}


SharedExp Unary::clone() const
{
    assert(subExp1);
    return std::make_shared<Unary>(m_oper, subExp1->clone());
}


bool Unary::operator==(const Exp& o) const
{
    if (o.getOper() == opWild) {
        return true;
    }

    if ((o.getOper() == opWildRegOf) && (m_oper == opRegOf)) {
        return true;
    }

    if ((o.getOper() == opWildMemOf) && (m_oper == opMemOf)) {
        return true;
    }

    if ((o.getOper() == opWildAddrOf) && (m_oper == opAddrOf)) {
        return true;
    }

    if (m_oper != o.getOper()) {
        return false;
    }

    return *subExp1 == *o.getSubExp1();
}


bool Unary::operator<(const Exp& o) const
{
    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    return *subExp1 < *((const Unary&)o).getSubExp1();
}


bool Unary::operator*=(const Exp& o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    if (other->getOper() == opWild) {
        return true;
    }

    if ((other->getOper() == opWildRegOf) && (m_oper == opRegOf)) {
        return true;
    }

    if ((other->getOper() == opWildMemOf) && (m_oper == opMemOf)) {
        return true;
    }

    if ((other->getOper() == opWildAddrOf) && (m_oper == opAddrOf)) {
        return true;
    }

    if (m_oper != other->getOper()) {
        return false;
    }

    return *subExp1 *= *other->getSubExp1();
}


void Unary::print(QTextStream& os, bool html) const
{
    SharedConstExp p1 = this->getSubExp1();

    switch (m_oper)
    {
    //    //    //    //    //    //    //
    //    x[ subexpression ]    //
    //    //    //    //    //    //    //
    case opRegOf:

        // Make a special case for the very common case of r[intConst]
        if (p1->isIntConst()) {
            os << "r" << std::static_pointer_cast<const Const>(p1)->getInt();
#ifdef DUMP_TYPES
            os << "T(" << std::static_pointer_cast<const Const>(p1)->getType() << ")";
#endif
            break;
        }
        else if (p1->isTemp()) {
            // Just print the temp {   // balance }s
            p1->print(os, html);
            break;
        }
        else {
            os << "r["; // e.g. r[r2]
            // Use print, not printr, because this is effectively the top level again (because the [] act as
            // parentheses)
            p1->print(os, html);
        }

        os << "]";
        break;

    case opMemOf:
    case opAddrOf:
    case opVar:
    case opTypeOf:
    case opKindOf:

        switch (m_oper)
        {
        case opMemOf:
            os << "m[";
            break;

        case opAddrOf:
            os << "a[";
            break;

        case opVar:
            os << "v[";
            break;

        case opTypeOf:
            os << "T[";
            break;

        case opKindOf:
            os << "K[";
            break;

        default:
            break; // Suppress compiler warning
        }

        if (m_oper == opVar) {
            std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
        }
        // Use print, not printr, because this is effectively the top level again (because the [] act as
        // parentheses)
        else {
            p1->print(os, html);
        }

        os << "]";
#ifdef DUMP_TYPES
        os << "T(" << std::static_pointer_cast<const Const>(p1)->getType() << ")";
#endif
        break;

    //    //    //    //    //    //    //
    //      Unary operators    //
    //    //    //    //    //    //    //

    case opNot:
    case opLNot:
    case opNeg:
    case opFNeg:

        if (m_oper == opNot) {
            os << "~";
        }
        else if (m_oper == opLNot) {
            os << "L~";
        }
        else if (m_oper == opFNeg) {
            os << "~f ";
        }
        else {
            os << "-";
        }

        p1->printr(os, html);
        return;

    case opSignExt:
        p1->printr(os, html);
        os << "!"; // Operator after expression
        return;

    //    //    //    //    //    //    //    //
    //    Function-like operators //
    //    //    //    //    //    //    //    //

    case opSQRTs:
    case opSQRTd:
    case opSQRTq:
    case opSqrt:
    case opSin:
    case opCos:
    case opTan:
    case opArcTan:
    case opLog2:
    case opLog10:
    case opLoge:
    case opPow:
    case opMachFtr:
    case opSuccessor:

        switch (m_oper)
        {
        case opSQRTs:
            os << "SQRTs(";
            break;

        case opSQRTd:
            os << "SQRTd(";
            break;

        case opSQRTq:
            os << "SQRTq(";
            break;

        case opSqrt:
            os << "sqrt(";
            break;

        case opSin:
            os << "sin(";
            break;

        case opCos:
            os << "cos(";
            break;

        case opTan:
            os << "tan(";
            break;

        case opArcTan:
            os << "arctan(";
            break;

        case opLog2:
            os << "log2(";
            break;

        case opLog10:
            os << "log10(";
            break;

        case opLoge:
            os << "loge(";
            break;

        case opExecute:
            os << "execute(";
            break;

        case opMachFtr:
            os << "machine(";
            break;

        case opSuccessor:
            os << "succ(";
            break;

        default:
            break; // For warning
        }

        p1->printr(os, html);
        os << ")";
        return;

    //    Misc    //
    case opSgnEx: // Different because the operator appears last
        p1->printr(os, html);
        os << "! ";
        return;

    case opTemp:

        if (p1->getOper() == opWildStrConst) {
            assert(p1->isTerminal());
            os << "t[";
            std::static_pointer_cast<const Terminal>(p1)->print(os);
            os << "]";
            return;
        }

    // fallthrough

    // Temp: just print the string, no quotes
    case opGlobal:
    case opLocal:
    case opParam:
        // Print a more concise form than param["foo"] (just foo)
        std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
        return;

    case opInitValueOf:
        p1->printr(os, html);
        os << "'";
        return;

    case opPhi:
        os << "phi(";
        p1->print(os, html);
        os << ")";
        return;

    case opFtrunc:
        os << "ftrunc(";
        p1->print(os, html);
        os << ")";
        return;

    case opFabs:
        os << "fabs(";
        p1->print(os, html);
        os << ")";
        return;

    default:
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }
}


void Unary::appendDotFile(QTextStream& of)
{
    // First a node for this Unary object
    of << "e_" << HostAddress(this).toString() << " [shape=record,label=\"{";
    // The (int) cast is to print the address, not the expression!
    of << operToString(m_oper) << "\\n" << HostAddress(this).toString() << " | ";
    of << "<p1>";
    of << " }\"];\n";

    // Now recurse to the subexpression.
    subExp1->appendDotFile(of);

    // Finally an edge for the subexpression
    of << "e_" << HostAddress(this) << "->e_" << HostAddress(subExp1.get()) << ";\n";
}


SharedExp Unary::match(const SharedConstExp& pattern)
{
    assert(subExp1);

    if (m_oper == pattern->getOper()) {
        return subExp1->match(pattern->getSubExp1());
    }

    return Exp::match(pattern);
}


bool Unary::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
    if (Exp::match(pattern, bindings)) {
        return true;
    }

#ifdef DEBUG_MATCH
    LOG_MSG("Matching %1 to %2.", this, pattern);
#endif

    if ((m_oper == opAddrOf) && pattern.startsWith("a[") && pattern.endsWith(']')) {
        return subExp1->match(pattern.mid(2, pattern.size() - 1), bindings); // eliminate 'a[' and ']'
    }

    return false;
}


void Unary::doSearchChildren(const Exp& pattern, std::list<SharedExp *>& li, bool once)
{
    if (m_oper != opInitValueOf) { // don't search child
        doSearch(pattern, subExp1, li, once);
    }
}


SharedExp Unary::simplifyArith()
{
    if ((m_oper == opMemOf) || (m_oper == opRegOf) || (m_oper == opAddrOf) || (m_oper == opSubscript)) {
        // assume we want to simplify the subexpression
        subExp1 = subExp1->simplifyArith();
    }

    return shared_from_this(); // Else, do nothing
}


SharedExp Unary::polySimplify(bool& bMod)
{
    SharedExp res(shared_from_this());

    subExp1 = subExp1->polySimplify(bMod);

    if ((m_oper == opNot) || (m_oper == opLNot)) {
        switch (subExp1->getOper())
        {
        case opEquals:
            res = res->getSubExp1();
            res->setOper(opNotEqual);
            bMod = true;
            return res;

        case opNotEqual:
            res = res->getSubExp1();
            res->setOper(opEquals);
            bMod = true;
            return res;

        case opLess:
            res = res->getSubExp1();
            res->setOper(opGtrEq);
            bMod = true;
            return res;

        case opLessEq:
            res = res->getSubExp1();
            res->setOper(opGtr);
            bMod = true;
            return res;

        case opGtr:
            res = res->getSubExp1();
            res->setOper(opLessEq);
            bMod = true;
            return res;

        case opGtrEq:
            res = res->getSubExp1();
            res->setOper(opLess);
            bMod = true;
            return res;

        case opLessUns:
            res = res->getSubExp1();
            res->setOper(opGtrEqUns);
            bMod = true;
            return res;

        case opLessEqUns:
            res = res->getSubExp1();
            res->setOper(opGtrUns);
            bMod = true;
            return res;

        case opGtrUns:
            res = res->getSubExp1();
            res->setOper(opLessEqUns);
            bMod = true;
            return res;

        case opGtrEqUns:
            res = res->getSubExp1();
            res->setOper(opLessUns);
            bMod = true;
            return res;

        default:
            break;
        }
    }

    switch (m_oper)
    {
    case opNeg:
    case opNot:
    case opLNot:
    case opSize:
        {
            OPER subOP = subExp1->getOper();

            if (subOP == opIntConst) {
                // -k, ~k, or !k
                OPER op2 = m_oper;
                res = res->getSubExp1();
                int k = std::static_pointer_cast<Const>(res)->getInt();

                switch (op2)
                {
                case opNeg:
                    k = -k;
                    break;

                case opNot:
                    k = ~k;
                    break;

                case opLNot:
                    k = !k;
                    break;

                case opSize: /* No change required */
                    break;

                default:
                    break;
                }

                std::static_pointer_cast<Const>(res)->setInt(k);
                bMod = true;
            }
            else if (m_oper == subOP) {
                res  = res->getSubExp1();
                res  = res->getSubExp1();
                bMod = true;
                break;
            }
        }
        break;

    case opAddrOf:

        // check for a[m[x]], becomes x
        if (subExp1->getOper() == opMemOf) {
            res  = res->getSubExp1();
            res  = res->getSubExp1();
            bMod = true;
            return res;
        }

        break;

    case opMemOf:
    case opRegOf:
        subExp1 = subExp1->polySimplify(bMod);
        // The below IS bad now. It undoes the simplification of
        // m[r29 + -4] to m[r29 - 4]
        // If really needed, do another polySimplify, or swap the order
        // subExp1 = subExp1->simplifyArith();        // probably bad
        break;

    default:
        break;
    }

    return res;
}


SharedExp Unary::simplifyAddr()
{
    SharedExp sub;

    if ((m_oper == opMemOf) && subExp1->isAddrOf()) {
        return getSubExp1()->getSubExp1();
    }

    if (m_oper != opAddrOf) {
        // Not a[ anything ]. Recurse
        subExp1 = subExp1->simplifyAddr();
        return shared_from_this();
    }

    if (subExp1->getOper() == opMemOf) {
        return getSubExp1()->getSubExp1();
    }

    if (subExp1->getOper() == opSize) {
        sub = subExp1->getSubExp2();

        if (sub->getOper() == opMemOf) {
            // Remove the a[
            auto b = getSubExp1();
            // Remove the size[
            auto u = b->getSubExp2();
            // Remove the m[
            return u->getSubExp1();
        }
    }

    // a[ something else ]. Still recurse, just in case
    subExp1 = subExp1->simplifyAddr();
    return shared_from_this();
}


void Unary::printx(int ind) const
{
    LOG_MSG("%1%2", QString(ind, ' '), operToString(m_oper));
    printChild(subExp1, ind);
}


SharedExp Unary::genConstraints(SharedExp result)
{
    if (result->isTypeVal()) {
        // TODO: need to check for conflicts
        return Terminal::get(opTrue);
    }

    switch (m_oper)
    {
    case opRegOf:
    case opParam: // Should be no params at constraint time
    case opGlobal:
    case opLocal:
        return Binary::get(opEquals, Unary::get(opTypeOf, this->clone()), result->clone());

    default:
        break;
    }

    return Terminal::get(opTrue);
}


SharedExp Unary::simplifyConstraint()
{
    subExp1 = subExp1->simplifyConstraint();
    return shared_from_this();
}


bool Unary::accept(ExpVisitor *v)
{
    bool override, ret = v->visit(shared_from_base<Unary>(), override);

    if (override || !ret) {
        return ret; // Override the rest of the accept logic
    }

    return subExp1->accept(v);
}


SharedExp Unary::accept(ExpModifier *v)
{
    // This Unary will be changed in *either* the pre or the post visit. If it's changed in the preVisit step, then
    // postVisit doesn't care about the type of ret. So let's call it a Unary, and the type system is happy
    bool recur = false;
    auto ret   = std::dynamic_pointer_cast<Unary>(v->preVisit(shared_from_base<Unary>(), recur));

    if (recur) {
        subExp1 = subExp1->accept(v);
    }

    assert(ret);
    return v->postVisit(ret);
}
