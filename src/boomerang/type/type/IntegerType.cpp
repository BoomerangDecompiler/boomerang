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


#include "boomerang/type/type/SizeType.h"
#include "boomerang/util/Log.h"


IntegerType::IntegerType(unsigned int NumBits, int sign)
    : Type(eInteger)
{
    size       = NumBits;
    signedness = sign;
}


std::shared_ptr<IntegerType> IntegerType::get(unsigned numBits, int sign)
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


bool IntegerType::operator==(const Type& other) const
{
    if (!other.isInteger()) {
        return false;
    }

    const IntegerType& otherInt = static_cast<const IntegerType &>(other);
    return
        // Note: zero size matches any other size (wild, or unknown, size)
        (size == 0 || otherInt.size == 0 || size == otherInt.size) &&
        // Note: actual value of signedness is disregarded, just whether less than, equal to, or greater than 0
        ((signedness < 0 && otherInt.signedness < 0) || (signedness == 0 && otherInt.signedness == 0) ||
         (signedness > 0 && otherInt.signedness > 0));
}


bool IntegerType::operator<(const Type& other) const
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
    switch (size)
    {
    case 1: /* Treat as a tmpb */
    case 8:
        return "tmpb";

    case 16:
        return "tmph";

    case 32:
        return "tmpi";

    case 64:
        return "tmpl";
    }

    return "tmp";
}


QString IntegerType::getCtype(bool final) const
{
    if (signedness >= 0) {
        QString s;

        if (!final && (signedness == 0)) {
            s = "/*signed?*/";
        }

        switch (size)
        {
        case 32:
            s += "int";
            break;

        case 16:
            s += "short";
            break;

        case 8:
            s += "char";
            break;

        case 1:
            s += "bool";
            break;

        case 64:
            s += "long long";
            break;

        default:

            if (!final) {
                s += "?"; // To indicate invalid/unknown size
            }

            s += "int";
        }

        return s;
    }
    else {
        switch (size)
        {
        case 32:
            return "unsigned int";

        case 16:
            return "unsigned short";

        case 8:
            return "unsigned char";

        case 1:
            return "bool";

        case 64:
            return "unsigned long long";

        default:

            if (final) {
                return "unsigned int";
            }
            else {
                return "?unsigned int";
            }
        }
    }
}


SharedType IntegerType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<IntegerType *>(this)->shared_from_this();
    }

    if (other->resolvesToInteger()) {
        std::shared_ptr<IntegerType> otherInt = other->as<IntegerType>();
        std::shared_ptr<IntegerType> result   = std::dynamic_pointer_cast<IntegerType>(this->clone());

        // Signedness
        if (otherInt->signedness > 0) {
            result->signedness++;
        }
        else if (otherInt->signedness < 0) {
            result->signedness--;
        }

        ch |= ((result->signedness > 0) != (signedness > 0)); // Changed from signed to not necessarily signed
        ch |= ((result->signedness < 0) != (signedness < 0)); // Changed from unsigned to not necessarily unsigned

        // Size. Assume 0 indicates unknown size
        result->size = std::max(size, otherInt->size);
        ch          |= (result->size != size);

        return result;
    }
    else if (other->resolvesToSize()) {
        std::shared_ptr<IntegerType> result   = std::dynamic_pointer_cast<IntegerType>(this->clone());
        std::shared_ptr<SizeType>    other_sz = other->as<SizeType>();

        if (size == 0) { // Doubt this will ever happen
            result->size = other_sz->getSize();
            ch           = true;
            return result;
        }

        if (size == other_sz->getSize()) {
            return result;
        }

        LOG_VERBOSE("Integer size %1 meet with SizeType size %2!", size, other_sz->getSize());

        result->size = std::max(size, other_sz->getSize());
        ch           = result->size != size;
        return result;
    }

    return createUnion(other, ch, bHighestPtr);
}



bool IntegerType::isCompatible(const Type& other, bool /*all*/) const
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
