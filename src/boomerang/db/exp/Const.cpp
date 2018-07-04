#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Const.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/CharType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"


Const::Const(uint32_t i)
    : Exp(opIntConst)
    , m_conscript(0)
    , m_type(VoidType::get())
{
    m_value.i = i;
}


Const::Const(int i)
    : Exp(opIntConst)
    , m_conscript(0)
    , m_type(VoidType::get())
{
    m_value.i = i;
}


Const::Const(QWord ll)
    : Exp(opLongConst)
    , m_conscript(0)
    , m_type(VoidType::get())
{
    m_value.ll = ll;
}


Const::Const(double d)
    : Exp(opFltConst)
    , m_conscript(0)
    , m_type(VoidType::get())
{
    m_value.d = d;
}


Const::Const(const QString& p)
    : Exp(opStrConst)
    , m_conscript(0)
    , m_type(VoidType::get())
{
    m_string = p;
}


Const::Const(Function *p)
    : Exp(opFuncConst)
    , m_conscript(0)
    , m_type(VoidType::get())
{
    m_value.pp = p;
}


Const::Const(Address addr)
    : Exp(opIntConst)
    , m_conscript(0)
    , m_type(VoidType::get())
{
    m_value.ll = addr.value();
}


Const::Const(const Const& other)
    : Exp(other.m_oper)
    , m_string(other.m_string)
    , m_conscript(other.m_conscript)
    , m_type(other.m_type)
{
    memcpy(&m_value, &other.m_value, sizeof(m_value));
}


bool Const::operator<(const Exp& o) const
{
    if (m_oper != o.getOper()) {
        return m_oper < o.getOper();
    }

    const Const& otherConst = static_cast<const Const&>(o);

    if (m_conscript != otherConst.m_conscript) {
        return m_conscript < otherConst.m_conscript;
    }

    switch (m_oper)
    {
    case opIntConst:
        return m_value.i < otherConst.m_value.i;

    case opLongConst:
        return m_value.ll < otherConst.m_value.ll;

    case opFltConst:
        return m_value.d < otherConst.m_value.d;

    case opStrConst:
        return m_string < otherConst.m_string;

    default:
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }

    return false;
}


bool Const::operator*=(const Exp& o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return *this == *other;
}


QString Const::getFuncName() const
{
    return m_value.pp->getName();
}


void Const::printx(int ind) const
{
    LOG_MSG("%1%2", QString(ind, ' '), operToString(m_oper));

    switch (m_oper)
    {
    case opIntConst:
        LOG_MSG("%1", m_value.i);
        break;

    case opFltConst:
        LOG_MSG("%1", m_value.d);
        break;

    case opStrConst:
        LOG_MSG("\"%1\"", m_string);
        break;

    case opFuncConst:
        LOG_MSG(m_value.pp->getName());
        break;

    default:
        LOG_MSG("?%1?", static_cast<int>(m_oper));
        break;
    }

    if (m_conscript) {
        LOG_MSG("\"%1\"", m_conscript);
    }
}


SharedExp Const::clone() const
{
    // Note: not actually cloning the Type* type pointer. Probably doesn't matter with GC
    return Const::get(*this);
}


void Const::print(QTextStream& os, bool) const
{
    switch (m_oper)
    {
    case opIntConst:
        if ((m_value.i < -1000) || (m_value.i > 1000)) {
            os << "0x" << QString::number(m_value.i, 16);
        }
        else {
            os << m_value.i;
        }
        break;

    case opLongConst:
        if ((static_cast<long long>(m_value.ll) < -1000LL) || (static_cast<long long>(m_value.ll) > 1000LL)) {
            os << "0x" << QString::number(m_value.ll, 16) << "LL";
        }
        else {
            os << m_value.ll << "LL";
        }
        break;

    case opFltConst:
        os << QString("%1").arg(m_value.d); // respects English locale
        break;

    case opStrConst:
        os << "\"" << m_string << "\"";
        break;

    default:
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }

    if (m_conscript) {
        os << "\\" << m_conscript << "\\";
    }

#ifdef DUMP_TYPES
    if (type) {
        os << "T(" << type->prints() << ")";
    }
#endif
}


void Const::printNoQuotes(QTextStream& os) const
{
    if (m_oper == opStrConst) {
        os << m_string;
    }
    else {
        print(os);
    }
}


bool Const::operator==(const Exp& other) const
{
    // Note: the casts of o to Const& are needed, else op is protected! Duh.
    if (other.getOper() == opWild) {
        return true;
    }

    if ((other.getOper() == opWildIntConst) && (m_oper == opIntConst)) {
        return true;
    }

    if ((other.getOper() == opWildStrConst) && (m_oper == opStrConst)) {
        return true;
    }

    if (m_oper != other.getOper()) {
        return false;
    }

    if ((m_conscript && (m_conscript != static_cast<const Const&>(other).m_conscript)) ||
        static_cast<const Const &>(other).m_conscript) {
            return false;
    }

    switch (m_oper)
    {
    case opIntConst:
        return m_value.i == static_cast<const Const &>(other).m_value.i;

    case opLongConst:
        return m_value.ll == static_cast<const Const &>(other).m_value.ll;

    case opFltConst:
        return m_value.d == static_cast<const Const &>(other).m_value.d;

    case opStrConst:
        return m_string == static_cast<const Const &>(other).m_string;

    default:
        LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }

    return false;
}


SharedType Const::ascendType()
{
    if (m_type->resolvesToVoid()) {
        switch (m_oper)
        {
            // could be anything, Boolean, Character, we could be bit fiddling pointers for all we know - trentw
        case opIntConst:  return VoidType::get();
        case opLongConst: return m_type = IntegerType::get(STD_SIZE * 2, Sign::Unknown);
        case opFltConst:  return m_type = FloatType::get(64);
        case opStrConst:  return m_type = PointerType::get(CharType::get());
        case opFuncConst: return m_type = PointerType::get(FuncType::get());
            // More needed here?
        default:
            assert(false); // Bad Const
        }
    }

    return m_type;
}


void Const::descendType(SharedType parentType, bool& changed, Statement *)
{
    bool thisCh = false;

    m_type = m_type->meetWith(parentType, thisCh);
    changed    |= thisCh;

    if (thisCh) {
        // May need to change the representation
        if (m_type->resolvesToFloat()) {
            if (m_oper == opIntConst) {
                m_oper = opFltConst;
                m_type = FloatType::get(64);
                float f = *reinterpret_cast<float *>(&m_value.i);
                m_value.d = static_cast<double>(f);
            }
            else if (m_oper == opLongConst) {
                m_oper = opFltConst;
                m_type = FloatType::get(64);
                m_value.d = *reinterpret_cast<double *>(&m_value.ll);
            }
        }

        // May be other cases
    }
}


bool Const::acceptVisitor(ExpVisitor *v)
{
    return v->visit(shared_from_base<Const>());
}


SharedExp Const::acceptPreModifier(ExpModifier* , bool& )
{
    return shared_from_this();
}


SharedExp Const::acceptPostModifier(ExpModifier* mod)
{
    return mod->postModify(access<Const>());
}
