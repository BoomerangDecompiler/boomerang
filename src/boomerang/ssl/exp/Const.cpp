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

#include "boomerang/db/proc/Proc.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


Const::Const(uint32_t i)
    : Exp(opIntConst)
    , m_type(VoidType::get())
{
    m_value = (int)i;
}


Const::Const(int i)
    : Exp(opIntConst)
    , m_type(VoidType::get())
{
    m_value = i;
}


Const::Const(QWord ll)
    : Exp(opLongConst)
    , m_type(VoidType::get())
{
    m_value = ll;
}


Const::Const(double d)
    : Exp(opFltConst)
    , m_type(VoidType::get())
{
    m_value = d;
}


Const::Const(const QString &p)
    : Exp(opStrConst)
    , m_type(VoidType::get())
{
    m_string = p;
}


Const::Const(Function *p)
    : Exp(opFuncConst)
    , m_type(VoidType::get())
{
    m_value = p;
}


Const::Const(Address addr)
    : Exp(opIntConst)
    , m_type(VoidType::get())
{
    m_value = (QWord)addr.value();
}


Const::Const(const Const &other)
    : Exp(other.m_oper)
    , m_value(other.m_value)
    , m_string(other.m_string)
    , m_type(other.m_type)
{
}


bool Const::operator<(const Exp &o) const
{
    if (m_oper != o.getOper()) {
        return m_oper < o.getOper();
    }

    const Const &otherConst = static_cast<const Const &>(o);

    switch (m_oper) {
    case opIntConst: return getInt() < otherConst.getInt();
    case opLongConst: return getLong() < otherConst.getLong();
    case opFltConst: return getFlt() < otherConst.getFlt();
    case opStrConst: return m_string < otherConst.m_string;

    default: LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }

    return false;
}


bool Const::operator*=(const Exp &o) const
{
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    return *this == *other;
}


int Const::getInt() const
{
    if (std::get_if<int>(&m_value) != nullptr) {
        return std::get<int>(m_value);
    }
    else if (std::get_if<QWord>(&m_value) != nullptr) {
        return (int)std::get<QWord>(m_value);
    }
    else {
        return (int)std::get<double>(m_value);
    }
}


QWord Const::getLong() const
{
    return std::get<QWord>(m_value);
}


double Const::getFlt() const
{
    return std::get<double>(m_value);
}


QString Const::getStr() const
{
    return m_string;
}


Address Const::getAddr() const
{
    if (std::get_if<QWord>(&m_value) != nullptr) {
        return Address(static_cast<Address::value_type>(std::get<QWord>(m_value)));
    }
    else {
        return Address(static_cast<Address::value_type>(std::get<int>(m_value)));
    }
}


QString Const::getFuncName() const
{
    return std::get<Function *>(m_value)->getName();
}


void Const::setInt(int value)
{
    m_value = value;
}


void Const::setLong(QWord value)
{
    m_value = value;
}


void Const::setFlt(double value)
{
    m_value = value;
}


void Const::setStr(const QString &value)
{
    m_string = value;
}


void Const::setAddr(Address addr)
{
    m_value = (QWord)addr.value();
}


SharedExp Const::clone() const
{
    // Note: not actually cloning the Type* type pointer. Probably doesn't matter with GC
    return Const::get(*this);
}


void Const::printNoQuotes(OStream &os) const
{
    if (m_oper == opStrConst) {
        os << m_string;
    }
    else {
        print(os);
    }
}


bool Const::operator==(const Exp &other) const
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

    const Const &otherConst = static_cast<const Const &>(other);

    switch (m_oper) {
    case opIntConst: return getInt() == otherConst.getInt();
    case opLongConst: return getLong() == otherConst.getLong();
    case opFltConst: return getFlt() == otherConst.getFlt();
    case opStrConst: return m_string == otherConst.m_string;
    default: LOG_FATAL("Invalid operator %1", operToString(m_oper));
    }

    return false;
}


SharedType Const::ascendType()
{
    if (m_type->resolvesToVoid()) {
        switch (m_oper) {
            // could be anything, Boolean, Character, we could be bit fiddling pointers for all we
            // know - trentw
        case opIntConst: return VoidType::get();
        case opLongConst: return m_type = IntegerType::get(STD_SIZE * 2, Sign::Unknown);
        case opFltConst: return m_type = FloatType::get(64);
        case opStrConst: return m_type = PointerType::get(CharType::get());
        case opFuncConst: return m_type = PointerType::get(FuncType::get());
        default: assert(false); // Bad Const
        }
    }

    return m_type;
}


void Const::descendType(SharedType parentType, bool &changed, Statement *)
{
    bool thisCh = false;

    m_type = m_type->meetWith(parentType, thisCh);
    changed |= thisCh;

    if (thisCh) {
        // May need to change the representation
        if (m_type->resolvesToFloat()) {
            if (m_oper == opIntConst) {
                m_oper  = opFltConst;
                m_type  = FloatType::get(64);
                int i   = getInt();
                float f = *reinterpret_cast<float *>(&i);
                m_value = static_cast<double>(f);
            }
            else if (m_oper == opLongConst) {
                m_oper  = opFltConst;
                m_type  = FloatType::get(64);
                QWord i = getLong();
                m_value = *reinterpret_cast<double *>(&i);
            }
        }

        // May be other cases
    }
}


bool Const::acceptVisitor(ExpVisitor *v)
{
    return v->visit(shared_from_base<Const>());
}


SharedExp Const::acceptPreModifier(ExpModifier *, bool &)
{
    return shared_from_this();
}


SharedExp Const::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Const>());
}
