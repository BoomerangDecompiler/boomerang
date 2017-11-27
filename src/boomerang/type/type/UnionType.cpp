#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UnionType.h"


#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/Log.h"


UnionType::UnionType()
    : Type(eUnion)
{
}


UnionType::UnionType::UnionType(const std::initializer_list<SharedType>& members)
    : Type(eUnion)
{
    for (SharedType member : members) {
        addType(member, "");
    }
}


UnionType::~UnionType()
{
}


SharedType UnionType::clone() const
{
    auto u = std::make_shared<UnionType>();

    for (UnionElement el : li) {
        u->addType(el.type, el.name);
    }

    return u;
}


size_t UnionType::getSize() const
{
    int max = 0;

    for (const UnionElement& elem : li) {
        int sz = elem.type->getSize();

        if (sz > max) {
            max = sz;
        }
    }

    return max;
}


bool UnionType::operator==(const Type& other) const
{
    if (!other.isUnion()) {
        return false;
    }

    const UnionType& uother = (UnionType&)other;

    if (uother.li.size() != li.size()) {
        return false;
    }

    for (const UnionElement& el : li) {
        if (uother.li.find(el) == uother.li.end()) {
            return false;
        }
    }

    return true;
}


bool UnionType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return getNumTypes() < ((const UnionType&)other).getNumTypes();
}


bool UnionType::findType(SharedType ty)
{
    UnionElement ue;

    ue.type = ty;
    return li.find(ue) != li.end();
}


void UnionType::addType(SharedType n, const QString& str)
{
    if (n->isUnion()) {
        auto utp = std::static_pointer_cast<UnionType>(n);
        // Note: need to check for name clashes eventually
        li.insert(utp->li.begin(), utp->li.end());
    }
    else {
        if (n->isPointer() && (n->as<PointerType>()->getPointsTo().get() == this)) { // Note: pointer comparison
            n = PointerType::get(VoidType::get());
            LOG_WARN("Attempt to union with pointer to self!");
        }

        UnionElement ue;
        ue.type = n;
        ue.name = str;
        li.insert(ue);
    }
}


QString UnionType::getCtype(bool final) const
{
    QString tmp("union { ");

    for (const UnionElement& el : li) {
        tmp += el.type->getCtype(final);

        if (el.name != "") {
            tmp += " ";
            tmp += el.name;
        }

        tmp += "; ";
    }

    tmp += "}";
    return tmp;
}
