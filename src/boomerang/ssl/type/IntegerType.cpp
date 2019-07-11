#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IntegerType.h"

#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/util/log/Log.h"


IntegerType::IntegerType(Size numBits, Sign sign)
    : Type(TypeClass::Integer)
    , m_size(numBits)
    , m_sign(sign)
{
}


std::shared_ptr<IntegerType> IntegerType::get(Size numBits, Sign sign)
{
    return std::make_shared<IntegerType>(numBits, sign);
}


SharedType IntegerType::clone() const
{
    return IntegerType::get(m_size, m_sign);
}


Type::Size IntegerType::getSize() const
{
    return m_size;
}


void IntegerType::hintAsSigned()
{
    m_sign = std::min((Sign)((int)m_sign + 1), Sign::SignedStrong);
}


void IntegerType::hintAsUnsigned()
{
    m_sign = std::max((Sign)((int)m_sign - 1), Sign::UnsignedStrong);
}


bool IntegerType::operator==(const Type &other) const
{
    if (!other.isInteger()) {
        return false;
    }

    const IntegerType &otherInt = static_cast<const IntegerType &>(other);

    return
        // Note: zero size matches any other size (wild, or unknown, size)
        (m_size == 0 || otherInt.m_size == 0 || m_size == otherInt.m_size) &&
        // Note: actual value of signedness is disregarded, just whether less than, equal to, or
        // greater than 0
        ((isUnsigned() && otherInt.isUnsigned()) || (isSignUnknown() && otherInt.isSignUnknown()) ||
         (isSigned() && otherInt.isSigned()));
}


bool IntegerType::operator<(const Type &other) const
{
    if (m_id != other.getId()) {
        return m_id < other.getId();
    }

    const IntegerType &otherInt = static_cast<const IntegerType &>(other);

    if (m_size != otherInt.m_size) {
        return m_size < otherInt.m_size;
    }

    // note: We cannot compare the sign directly here, because otherwise e.g.
    //  IntegerType(32, Sign::Signed) == IntegerType(32, Sign::SignedStrong)
    // would be true, while
    //  IntegerType(32, Sign::Signed) <  IntegerType(32, Sign::SignedStrong)
    // would also be true.
    return (isUnsigned() && otherInt.isMaybeSigned()) || (isSignUnknown() && otherInt.isSigned());
}


bool IntegerType::isComplete()
{
    return m_sign != Sign::Unknown && m_size != 0;
}


QString IntegerType::getCtype(bool final) const
{
    if (isMaybeSigned()) {
        QString s;

        if (!final && isSignUnknown()) {
            s = "/*signed?*/";
        }

        switch (m_size) {
        case 1: return s + "bool";
        case 8: return s + "char";
        case 16: return s + "short";
        case 32: return s + "int";
        case 64: return s + "long long";
        default: return s + (final ? "int" : "?int"); // To indicate invalid/unknown size
        }
    }
    else {
        switch (m_size) {
        case 1: return "bool";
        case 8: return "unsigned char";
        case 16: return "unsigned short";
        case 32: return "unsigned int";
        case 64: return "unsigned long long";
        default: return final ? "unsigned int" : "?unsigned int";
        }
    }
}


SharedType IntegerType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid() || other->resolvesToChar()) {
        return const_cast<IntegerType *>(this)->shared_from_this();
    }

    if (other->resolvesToInteger()) {
        std::shared_ptr<IntegerType> otherInt = other->as<IntegerType>();
        std::shared_ptr<IntegerType> result   = this->clone()->as<IntegerType>();

        // Signedness
        if (otherInt->isSigned()) {
            result->hintAsSigned();
        }
        else if (otherInt->isUnsigned()) {
            result->hintAsUnsigned();
        }

        // Changed from signed to not necessarily signed
        changed |= result->isSigned() != isSigned();
        // Changed from unsigned to not necessarily unsigned
        changed |= result->isUnsigned() != isUnsigned();

        // Size. Assume 0 indicates unknown size
        result->m_size = std::max(m_size, otherInt->m_size);
        changed |= (result->m_size != m_size);

        return result;
    }
    else if (other->resolvesToSize()) {
        std::shared_ptr<IntegerType> result = std::dynamic_pointer_cast<IntegerType>(this->clone());
        std::shared_ptr<SizeType> other_sz  = other->as<SizeType>();

        if (m_size == 0) { // Doubt this will ever happen
            result->m_size = other_sz->getSize();
            changed        = true;
            return result;
        }

        if (m_size == other_sz->getSize()) {
            return result;
        }

        LOG_VERBOSE("Integer size %1 meet with SizeType size %2!", m_size, other_sz->getSize());

        result->m_size = std::max(m_size, other_sz->getSize());
        changed        = result->m_size != m_size;
        return result;
    }

    return createUnion(other, changed, useHighestPtr);
}


bool IntegerType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.resolvesToVoid() || other.resolvesToInteger() || other.resolvesToChar()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == m_size) {
        return true;
    }

    return false;
}
