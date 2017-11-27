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

    IntegerType& otherInt = (IntegerType&)other;
    return
        // Note: zero size matches any other size (wild, or unknown, size)
        (size == 0 || otherInt.size == 0 || size == otherInt.size) &&
        // Note: actual value of signedness is disregarded, just whether less than, equal to, or greater than 0
        ((signedness < 0 && otherInt.signedness < 0) || (signedness == 0 && otherInt.signedness == 0) ||
         (signedness > 0 && otherInt.signedness > 0));
}


bool IntegerType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    if (size < ((IntegerType&)other).size) {
        return true;
    }

    if (size > ((IntegerType&)other).size) {
        return false;
    }

    return(signedness < ((IntegerType&)other).signedness);
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
