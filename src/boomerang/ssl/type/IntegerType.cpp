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


IntegerType::IntegerType(unsigned int NumBits, Sign sign)
    : Type(TypeClass::Integer)
{
    size       = NumBits;
    signedness = sign;
}


std::shared_ptr<IntegerType> IntegerType::get(unsigned numBits, Sign sign)
{
    return std::make_shared<IntegerType>(numBits, sign);
}


SharedType IntegerType::clone() const
{
    return IntegerType::get(size, signedness);
}


size_t IntegerType::getSize() const
{
    return size;
}

void IntegerType::hintAsSigned()
{
    signedness = std::min((Sign)((int)signedness + 1), Sign::SignedStrong);
}


void IntegerType::hintAsUnsigned()
{
    signedness = std::max((Sign)((int)signedness - 1), Sign::UnsignedStrong);
}


bool IntegerType::operator==(const Type &other) const
{
    if (!other.isInteger()) {
        return false;
    }

    const IntegerType &otherInt = static_cast<const IntegerType &>(other);

    return
        // Note: zero size matches any other size (wild, or unknown, size)
        (size == 0 || otherInt.size == 0 || size == otherInt.size) &&
        // Note: actual value of signedness is disregarded, just whether less than, equal to, or
        // greater than 0
        ((isUnsigned() && otherInt.isUnsigned()) || (isSignUnknown() && otherInt.isSignUnknown()) ||
         (isSigned() && otherInt.isSigned()));
}


bool IntegerType::operator<(const Type &other) const
{
    if (id != other.getId()) {
        return id < other.getId();
    }

    const IntegerType &otherTy = static_cast<const IntegerType &>(other);

    if (size != otherTy.size) {
        return size < otherTy.size;
    }

    return signedness < otherTy.signedness;
}


QString IntegerType::getTempName() const
{
    switch (size) {
    case 1: /* Treat as a tmpb */
    case 8: return "tmpb";
    case 16: return "tmph";
    case 32: return "tmpi";
    case 64: return "tmpl";
    }

    return "tmp";
}


QString IntegerType::getCtype(bool final) const
{
    if (isMaybeSigned()) {
        QString s;

        if (!final && isSignUnknown()) {
            s = "/*signed?*/";
        }

        switch (size) {
        case 1: return s + "bool";
        case 8: return s + "char";
        case 16: return s + "short";
        case 32: return s + "int";
        case 64: return s + "long long";
        default: return s + (final ? "int" : "?int"); // To indicate invalid/unknown size
        }
    }
    else {
        switch (size) {
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
    if (other->resolvesToVoid()) {
        return const_cast<IntegerType *>(this)->shared_from_this();
    }

    if (other->resolvesToInteger()) {
        std::shared_ptr<IntegerType> otherInt = other->as<IntegerType>();
        std::shared_ptr<IntegerType> result = std::dynamic_pointer_cast<IntegerType>(this->clone());

        // Signedness
        if (otherInt->isSigned()) {
            result->hintAsSigned();
        }
        else if (otherInt->isUnsigned()) {
            result->hintAsUnsigned();
        }

        changed |= result->isSigned() !=
                   isSigned(); // Changed from signed to not necessarily signed
        changed |= result->isUnsigned() !=
                   isUnsigned(); // Changed from unsigned to not necessarily unsigned

        // Size. Assume 0 indicates unknown size
        result->size = std::max(size, otherInt->size);
        changed |= (result->size != size);

        return result;
    }
    else if (other->resolvesToSize()) {
        std::shared_ptr<IntegerType> result = std::dynamic_pointer_cast<IntegerType>(this->clone());
        std::shared_ptr<SizeType> other_sz  = other->as<SizeType>();

        if (size == 0) { // Doubt this will ever happen
            result->size = other_sz->getSize();
            changed      = true;
            return result;
        }

        if (size == other_sz->getSize()) {
            return result;
        }

        LOG_VERBOSE("Integer size %1 meet with SizeType size %2!", size, other_sz->getSize());

        result->size = std::max(size, other_sz->getSize());
        changed      = result->size != size;
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

    if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == size) {
        return true;
    }

    return false;
}
