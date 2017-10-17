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


#include "boomerang/util/Log.h"


static int nextAlpha = 0;


NamedType::NamedType(const QString& _name)
    : Type(eNamed)
    , name(_name)
{
}


NamedType::~NamedType()
{
}


SharedType NamedType::clone() const
{
    return NamedType::get(name);
}


size_t NamedType::getSize() const
{
    SharedType ty = resolvesTo();

    if (ty) {
        return ty->getSize();
    }

    LOG_WARN("Unknown size for named type %1", name);
    return 0; // don't know
}


bool NamedType::operator==(const Type& other) const
{
    return other.isNamed() && (name == ((NamedType&)other).name);
}


SharedType NamedType::resolvesTo() const
{
    SharedType ty = getNamedType(name);

    if (ty && ty->isNamed()) {
        return std::static_pointer_cast<NamedType>(ty)->resolvesTo();
    }

    return ty;
}


bool NamedType::operator<(const Type& other) const
{
    if (id > other.getId()) {
        return false;
    }

    return id < other.getId() || (name < ((NamedType&)other).name);
}


SharedExp NamedType::match(SharedType pattern)
{
    return Type::match(pattern);
}


QString NamedType::getCtype(bool /*final*/) const
{
    return name;
}


std::shared_ptr<NamedType> NamedType::getAlpha()
{
    return NamedType::get(QString("alpha%1").arg(nextAlpha++));
}
