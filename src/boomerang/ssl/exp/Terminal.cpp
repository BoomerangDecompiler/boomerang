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

#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


Terminal::Terminal(OPER _op)
    : Exp(_op)
{
}


Terminal::Terminal(const Terminal &o)
    : Exp(o.m_oper)
{
}


SharedExp Terminal::get(OPER op)
{
    return std::make_shared<Terminal>(op);
}


SharedExp Terminal::clone() const
{
    return std::make_shared<Terminal>(*this);
}


bool Terminal::operator==(const Exp &o) const
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

    return ((m_oper == opWild) || // Wild matches anything
            (o.getOper() == opWild) || (m_oper == o.getOper()));
}


bool Terminal::operator<(const Exp &o) const
{
    return (m_oper < o.getOper());
}


bool Terminal::equalNoSubscript(const Exp &o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return *this == *other;
}


bool Terminal::acceptVisitor(ExpVisitor *v)
{
    return v->visit(shared_from_base<Terminal>());
}


SharedType Terminal::ascendType()
{
    // Can also find various terminals at the leaves of an expression tree
    switch (m_oper) {
    case opPC: return IntegerType::get(STD_SIZE, Sign::Unsigned);
    case opCF:
    case opZF:
    case opFZF:
    case opFLF:
    case opTrue:
    case opFalse: return BooleanType::get();
    case opDefineAll: return VoidType::get();
    case opFlags: return IntegerType::get(STD_SIZE, Sign::Unsigned);
    default: LOG_WARN("Unknown type %1", shared_from_this()); return VoidType::get();
    }
}


bool Terminal::descendType(SharedType)
{
    return false;
}


SharedExp Terminal::acceptPreModifier(ExpModifier *, bool &)
{
    // return mod->preModify(access<Terminal>(), visitChildren));
    return access<Terminal>();
}


SharedExp Terminal::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Terminal>());
}
