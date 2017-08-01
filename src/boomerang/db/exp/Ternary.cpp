#include "Ternary.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/exp/TypeVal.h"
#include "boomerang/db/Visitor.h"
#include "boomerang/util/Log.h"


Ternary::Ternary(OPER _op)
    : Binary(_op)
{
    subExp3 = nullptr;
}


Ternary::Ternary(OPER _op, SharedExp _e1, SharedExp _e2, SharedExp _e3)
    : Binary(_op, _e1, _e2)
{
    subExp3 = _e3;
    assert(subExp1 && subExp2 && subExp3);
}


Ternary::Ternary(const Ternary& o)
    : Binary(o.m_oper)
{
    subExp1 = o.subExp1->clone();
    subExp2 = o.subExp2->clone();
    subExp3 = o.subExp3->clone();
    assert(subExp1 && subExp2 && subExp3);
}


Ternary::~Ternary()
{
    if (subExp3 != nullptr) {
        // delete subExp3;
    }
}


void Ternary::setSubExp3(SharedExp e)
{
    if (subExp3 != nullptr) {
        // delete subExp3;
    }

    subExp3 = e;
    assert(subExp1 && subExp2 && subExp3);
}


SharedExp Ternary::getSubExp3()
{
    assert(subExp1 && subExp2 && subExp3);
    return subExp3;
}


SharedConstExp Ternary::getSubExp3() const
{
    assert(subExp1 && subExp2 && subExp3);
    return subExp3;
}


SharedExp& Ternary::refSubExp3()
{
    assert(subExp1 && subExp2 && subExp3);
    return subExp3;
}


SharedExp Ternary::clone() const
{
    assert(subExp1 && subExp2 && subExp3);
    std::shared_ptr<Ternary> c = std::make_shared<Ternary>(m_oper, subExp1->clone(), subExp2->clone(), subExp3->clone());
    return c;
}


bool Ternary::operator==(const Exp& o) const
{
    if (o.getOper() == opWild) {
        return true;
    }

    if (nullptr == dynamic_cast<const Ternary *>(&o)) {
        return false;
    }

    if (m_oper != ((Ternary&)o).m_oper) {
        return false;
    }

    if (!(*subExp1 == *((Ternary&)o).getSubExp1())) {
        return false;
    }

    if (!(*subExp2 == *((Ternary&)o).getSubExp2())) {
        return false;
    }

    return *subExp3 == *((Ternary&)o).getSubExp3();
}



bool Ternary::operator<(const Exp& o) const
{
    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    if (*subExp1 < *((Ternary&)o).getSubExp1()) {
        return true;
    }

    if (*((Ternary&)o).getSubExp1() < *subExp1) {
        return false;
    }

    if (*subExp2 < *((Ternary&)o).getSubExp2()) {
        return true;
    }

    if (*((Ternary&)o).getSubExp2() < *subExp2) {
        return false;
    }

    return *subExp3 < *((Ternary&)o).getSubExp3();
}


bool Ternary::operator*=(const Exp& o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    if (other->getOper() == opWild) {
        return true;
    }

    if (m_oper != other->getOper()) {
        return false;
    }

    if (!(*subExp1 *= *other->getSubExp1())) {
        return false;
    }

    if (!(*subExp2 *= *other->getSubExp2())) {
        return false;
    }

    return *subExp3 *= *other->getSubExp3();
}
void Ternary::printr(QTextStream& os, bool) const
{
    // The function-like operators don't need parentheses
    switch (m_oper)
    {
    // The "function-like" ternaries
    case opTruncu:
    case opTruncs:
    case opZfill:
    case opSgnEx:
    case opFsize:
    case opItof:
    case opFtoi:
    case opFround:
    case opFtrunc:
    case opOpTable:
        // No paren case
        print(os);
        return;

    default:
        break;
    }

    // All other cases, we use the parens
    os << "(" << this << ")";
}


void Ternary::print(QTextStream& os, bool html) const
{
    SharedConstExp p1 = this->getSubExp1();
    SharedConstExp p2 = this->getSubExp2();
    SharedConstExp p3 = this->getSubExp3();

    switch (m_oper)
    {
    // The "function-like" ternaries
    case opTruncu:
    case opTruncs:
    case opZfill:
    case opSgnEx:
    case opFsize:
    case opItof:
    case opFtoi:
    case opFround:
    case opFtrunc:
    case opOpTable:

        switch (m_oper)
        {
        case opTruncu:
            os << "truncu(";
            break;

        case opTruncs:
            os << "truncs(";
            break;

        case opZfill:
            os << "zfill(";
            break;

        case opSgnEx:
            os << "sgnex(";
            break;

        case opFsize:
            os << "fsize(";
            break;

        case opItof:
            os << "itof(";
            break;

        case opFtoi:
            os << "ftoi(";
            break;

        case opFround:
            os << "fround(";
            break;

        case opFtrunc:
            os << "ftrunc(";
            break;

        case opOpTable:
            os << "optable(";
            break;

        default:
            break; // For warning
        }

        // Use print not printr here, since , has the lowest precendence of all.
        // Also it makes it the same as UQBT, so it's easier to test
        if (p1) {
            p1->print(os, html);
        }
        else {
            os << "<nullptr>";
        }

        os << ",";

        if (p2) {
            p2->print(os, html);
        }
        else {
            os << "<nullptr>";
        }

        os << ",";

        if (p3) {
            p3->print(os, html);
        }
        else {
            os << "<nullptr>";
        }

        os << ")";
        return;

    default:
        break;
    }

    // Else must be ?: or @ (traditional ternary operators)
    if (p1) {
        p1->printr(os, html);
    }
    else {
        os << "<nullptr>";
    }

    if (m_oper == opTern) {
        os << " ? ";

        if (p2) {
            p2->printr(os, html);
        }
        else {
            os << "<nullptr>";
        }

        os << " : "; // Need wide spacing here

        if (p3) {
            p3->print(os, html);
        }
        else {
            os << "<nullptr>";
        }
    }
    else if (m_oper == opAt) {
        os << "@";

        if (p2) {
            p2->printr(os, html);
        }
        else {
            os << "nullptr>";
        }

        os << ":";

        if (p3) {
            p3->printr(os, html);
        }
        else {
            os << "nullptr>";
        }
    }
    else {
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }
}


void Ternary::appendDotFile(QTextStream& of)
{
    // First a node for this Ternary object
    of << "e_" << HostAddress(this) << " [shape=record,label=\"{";
    of << operToString(m_oper) << "\\n" << HostAddress(this) << " | ";
    of << "{<p1> | <p2> | <p3>}";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    subExp2->appendDotFile(of);
    subExp3->appendDotFile(of);
    // Now an edge for each subexpression
    of << "e_" << HostAddress(this) << ":p1->e_" << HostAddress(subExp1.get()) << ";\n";
    of << "e_" << HostAddress(this) << ":p2->e_" << HostAddress(subExp2.get()) << ";\n";
    of << "e_" << HostAddress(this) << ":p3->e_" << HostAddress(subExp3.get()) << ";\n";
}


bool Ternary::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
    if (Exp::match(pattern, bindings)) {
        return true;
    }

#ifdef DEBUG_MATCH
    LOG_MSG("Matching %1 to %2.", this, pattern);
#endif
    return false;
}


void Ternary::doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once)
{
    doSearch(search, subExp1, li, once);

    if (once && li.size()) {
        return;
    }

    doSearch(search, subExp2, li, once);

    if (once && li.size()) {
        return;
    }

    doSearch(search, subExp3, li, once);
}


SharedExp Ternary::simplifyArith()
{
    subExp1 = subExp1->simplifyArith();
    subExp2 = subExp2->simplifyArith();
    subExp3 = subExp3->simplifyArith();
    return shared_from_this();
}


SharedExp Ternary::polySimplify(bool& bMod)
{
    SharedExp res = shared_from_this();

    subExp1 = subExp1->polySimplify(bMod);
    subExp2 = subExp2->polySimplify(bMod);
    subExp3 = subExp3->polySimplify(bMod);

    // p ? 1 : 0 -> p
    if ((m_oper == opTern) && (subExp2->getOper() == opIntConst) && (subExp3->getOper() == opIntConst)) {
        auto s2 = std::static_pointer_cast<Const>(subExp2);
        auto s3 = std::static_pointer_cast<Const>(subExp3);

        if ((s2->getInt() == 1) && (s3->getInt() == 0)) {
            res  = getSubExp1();
            bMod = true;
            return res;
        }
    }

    // 1 ? x : y -> x
    if ((m_oper == opTern) && (subExp1->getOper() == opIntConst) && (std::static_pointer_cast<const Const>(subExp1)->getInt() == 1)) {
        res  = this->getSubExp2();
        bMod = true;
        return res;
    }

    // 0 ? x : y -> y
    if ((m_oper == opTern) && (subExp1->getOper() == opIntConst) && (std::static_pointer_cast<const Const>(subExp1)->getInt() == 0)) {
        res  = this->getSubExp3();
        bMod = true;
        return res;
    }

    if (((m_oper == opSgnEx) || (m_oper == opZfill)) && (subExp3->getOper() == opIntConst)) {
        res  = this->getSubExp3();
        bMod = true;
        return res;
    }

    if ((m_oper == opFsize) && (subExp3->getOper() == opItof) && (*subExp1 == *subExp3->getSubExp2()) &&
        (*subExp2 == *subExp3->getSubExp1())) {
        res  = this->getSubExp3();
        bMod = true;
        return res;
    }

    if ((m_oper == opFsize) && (subExp3->getOper() == opFltConst)) {
        res  = this->getSubExp3();
        bMod = true;
        return res;
    }

    if ((m_oper == opItof) && (subExp3->getOper() == opIntConst) && (subExp2->getOper() == opIntConst) &&
        (std::static_pointer_cast<const Const>(subExp2)->getInt() == 32)) {
        unsigned n = std::static_pointer_cast<const Const>(subExp3)->getInt();
        res  = Const::get(*(float *)&n);
        bMod = true;
        return res;
    }

    if ((m_oper == opFsize) && (subExp3->getOper() == opMemOf) && (subExp3->getSubExp1()->getOper() == opIntConst)) {
        Address  u  = subExp3->access<Const, 1>()->getAddr();
        auto     l  = std::dynamic_pointer_cast<Location>(subExp3);
        UserProc *p = l->getProc();

        if (p) {
            Prog   *prog = p->getProg();
            bool   ok;
            double d = prog->getFloatConstant(u, ok, std::static_pointer_cast<const Const>(subExp1)->getInt());

            if (ok) {
                LOG_VERBOSE("Replacing %1 with %2 in %3", subExp3, d, shared_from_this());

                subExp3 = Const::get(d);
                bMod    = true;
                return res;
            }
        }
    }

    if ((m_oper == opTruncu) && subExp3->isIntConst()) {
        int          from = std::static_pointer_cast<const Const>(subExp1)->getInt();
        int          to   = std::static_pointer_cast<const Const>(subExp2)->getInt();
        unsigned int val  = std::static_pointer_cast<const Const>(subExp3)->getInt();

        if (from == 32) {
            if (to == 16) {
                res  = Const::get(Address(val & 0xffff));
                bMod = true;
                return res;
            }

            if (to == 8) {
                res  = Const::get(Address(val & 0xff));
                bMod = true;
                return res;
            }
        }
    }

    if ((m_oper == opTruncs) && subExp3->isIntConst()) {
        int from = std::static_pointer_cast<const Const>(subExp1)->getInt();
        int to   = std::static_pointer_cast<const Const>(subExp2)->getInt();
        int val  = std::static_pointer_cast<const Const>(subExp3)->getInt();

        if (from == 32) {
            if (to == 16) {
                res  = Const::get(val & 0xffff);
                bMod = true;
                return res;
            }

            if (to == 8) {
                res  = Const::get(val & 0xff);
                bMod = true;
                return res;
            }
        }
    }

    return res;
}


SharedExp Ternary::simplifyAddr()
{
    subExp1 = subExp1->simplifyAddr();
    subExp2 = subExp2->simplifyAddr();
    subExp3 = subExp3->simplifyAddr();
    return shared_from_this();
}


SharedExp Ternary::genConstraints(SharedExp result)
{
    SharedType argHasToBe = nullptr;
    SharedType retHasToBe = nullptr;

    switch (m_oper)
    {
    case opFsize:
    case opItof:
    case opFtoi:
    case opSgnEx:
        {
            assert(subExp1->isIntConst());
            assert(subExp2->isIntConst());
            int fromSize = std::static_pointer_cast<const Const>(subExp1)->getInt();
            int toSize   = std::static_pointer_cast<const Const>(subExp2)->getInt();

            // Fall through
            switch (m_oper)
            {
            case opFsize:
                argHasToBe = FloatType::get(fromSize);
                retHasToBe = FloatType::get(toSize);
                break;

            case opItof:
                argHasToBe = IntegerType::get(fromSize);
                retHasToBe = FloatType::get(toSize);
                break;

            case opFtoi:
                argHasToBe = FloatType::get(fromSize);
                retHasToBe = IntegerType::get(toSize);
                break;

            case opSgnEx:
                argHasToBe = IntegerType::get(fromSize);
                retHasToBe = IntegerType::get(toSize);
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    SharedExp res = nullptr;

    if (retHasToBe) {
        if (result->isTypeVal()) {
            // result is a constant type, or possibly a partial type such as
            // ptr(alpha)
            SharedType t = result->access<TypeVal>()->getType();

            // Compare broad types
            if (!(*retHasToBe *= *t)) {
                return Terminal::get(opFalse);
            }

            // else just constrain the arg
        }
        else {
            // result is a type variable, constrained by this Ternary
            res = Binary::get(opEquals, result, TypeVal::get(retHasToBe));
        }
    }

    if (argHasToBe) {
        // Constrain the argument
        SharedExp con = subExp3->genConstraints(TypeVal::get(argHasToBe));

        if (res) {
            res = Binary::get(opAnd, res, con);
        }
        else {
            res = con;
        }
    }

    if (res == nullptr) {
        return Terminal::get(opTrue);
    }

    return res;
}


bool Ternary::accept(ExpVisitor *v)
{
    bool override, ret = v->visit(shared_from_base<Ternary>(), override);

    if (override) {
        return ret;
    }

    if (ret) {
        ret = subExp1->accept(v);
    }

    if (ret) {
        ret = subExp2->accept(v);
    }

    if (ret) {
        ret = subExp3->accept(v);
    }

    return ret;
}


SharedExp Ternary::accept(ExpModifier *v)
{
    bool recur;
    auto ret = std::static_pointer_cast<Ternary>(v->preVisit(shared_from_base<Ternary>(), recur));

    if (recur) {
        subExp1 = subExp1->accept(v);
    }

    if (recur) {
        subExp2 = subExp2->accept(v);
    }

    if (recur) {
        subExp3 = subExp3->accept(v);
    }

    assert(std::dynamic_pointer_cast<Ternary>(ret));
    return v->postVisit(ret);
}

void Ternary::printx(int ind) const
{
    LOG_MSG("%1%2", QString(ind, ' '), operToString(m_oper));

    child(subExp1, ind);
    child(subExp2, ind);
    child(subExp3, ind);
}

