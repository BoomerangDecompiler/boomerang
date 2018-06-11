#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Terminal.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/type/type/BooleanType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/Log.h"


Terminal::Terminal(OPER _op)
    : Exp(_op)
{
}


Terminal::Terminal(const Terminal& o)
    : Exp(o.m_oper)
{
}


SharedExp Terminal::clone() const
{
    return std::make_shared<Terminal>(*this);
}


bool Terminal::operator==(const Exp& o) const
{
    if (m_oper == opWildIntConst) {
        return o.getOper() == opIntConst;
    }

    if (m_oper == opWildStrConst) {
        return o.getOper() == opStrConst;
    }

    if (m_oper == opWildMemOf) {
        return o.getOper() == opMemOf;
    }

    if (m_oper == opWildRegOf) {
        return o.getOper() == opRegOf;
    }

    if (m_oper == opWildAddrOf) {
        return o.getOper() == opAddrOf;
    }

    return((m_oper == opWild) ||  // Wild matches anything
           (o.getOper() == opWild) || (m_oper == o.getOper()));
}


bool Terminal::operator<(const Exp& o) const
{
    return(m_oper < o.getOper());
}


bool Terminal::operator*=(const Exp& o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return *this == *other;
}


void Terminal::print(QTextStream& os, bool) const
{
    switch (m_oper)
    {
    case opPC:
        os << "%pc";
        break;

    case opFlags:
        os << "%flags";
        break;

    case opFflags:
        os << "%fflags";
        break;

    case opCF:
        os << "%CF";
        break;

    case opZF:
        os << "%ZF";
        break;

    case opOF:
        os << "%OF";
        break;

    case opNF:
        os << "%NF";
        break;

    case opDF:
        os << "%DF";
        break;

    case opAFP:
        os << "%afp";
        break;

    case opAGP:
        os << "%agp";
        break;

    case opWild:
        os << "WILD";
        break;

    case opAnull:
        os << "%anul";
        break;

    case opFpush:
        os << "FPUSH";
        break;

    case opFpop:
        os << "FPOP";
        break;

    case opWildMemOf:
        os << "m[WILD]";
        break;

    case opWildRegOf:
        os << "r[WILD]";
        break;

    case opWildAddrOf:
        os << "a[WILD]";
        break;

    case opWildIntConst:
        os << "WILDINT";
        break;

    case opWildStrConst:
        os << "WILDSTR";
        break;

    case opNil:
        break;

    case opTrue:
        os << "true";
        break;

    case opFalse:
        os << "false";
        break;

    case opDefineAll:
        os << "<all>";
        break;

    default:
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }
}


bool Terminal::accept(ExpVisitor *v)
{
    return v->visit(shared_from_base<Terminal>());
}


SharedExp Terminal::accept(ExpModifier *v)
{
    // This is important if we need to modify terminals
    SharedExp val      = v->preModify(shared_from_base<Terminal>());
    auto      term_res = std::dynamic_pointer_cast<Terminal>(val);

    if (term_res) {
        return v->postModify(term_res);
    }

    auto ref_res = std::dynamic_pointer_cast<RefExp>(val);

    if (ref_res) {
        return v->postModify(ref_res);
    }

    assert(false);
    return nullptr;
}


void Terminal::printx(int ind) const
{
    LOG_MSG("%1%2", QString(ind, ' '), operToString(m_oper));
}


SharedType Terminal::ascendType()
{
    // Can also find various terminals at the leaves of an expression tree
    switch (m_oper)
    {
    case opPC:
        return IntegerType::get(STD_SIZE, -1);

    case opCF:
    case opZF:
        return BooleanType::get();

    case opDefineAll:
        return VoidType::get();

    case opFlags:
        return IntegerType::get(STD_SIZE, -1);

    default:
        LOG_WARN("Unknown type %1", this->toString());
        return VoidType::get();
    }
}

void Terminal::descendType(SharedType, bool& changed, Statement*)
{
    changed = false;
}

