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


#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"

#include <QHash>


size_t hashUnionElem::operator()(const UnionElement& e) const
{
        return qHash(e.type->getCtype());
}


UnionType::UnionType()
    : Type(TypeClass::Union)
{
}


UnionType::UnionType::UnionType(const std::initializer_list<SharedType>& members)
    : Type(TypeClass::Union)
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

    const UnionType& uother = static_cast<const UnionType &>(other);

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
    if (id != other.getId()) {
        return id < other.getId();
    }

    return getNumTypes() < static_cast<const UnionType &>(other).getNumTypes();
}


bool UnionType::findType(SharedType ty)
{
    UnionElement ue;

    ue.type = ty;
    return li.find(ue) != li.end();
}


void UnionType::addType(SharedType n, const QString& name)
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
        ue.name = name;
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


#ifdef PRINT_UNION
unsigned unionCount = 0;
#endif

static int nextUnionNumber = 0;

SharedType UnionType::meetWith(SharedType other, bool& changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<UnionType *>(this)->shared_from_this();
    }

    if (other->resolvesToUnion()) {
        if (this == other.get()) {                          // Note: pointer comparison
            return const_cast<UnionType *>(this)->shared_from_this(); // Avoid infinite recursion
        }

        std::shared_ptr<UnionType> otherUnion = other->as<UnionType>();
        std::shared_ptr<UnionType> result(UnionType::get());

        *result = *this;

        for (UnionElement elem : otherUnion->li) {
            result = result->meetWith(elem.type, changed, useHighestPtr)->as<UnionType>();
        }

        return result;
    }

    // Other is a non union type
    if (other->resolvesToPointer() && (other->as<PointerType>()->getPointsTo().get() == this)) {
        LOG_WARN("Attempt to union '%1' with pointer to self!", this->getCtype());
        return const_cast<UnionType *>(this)->shared_from_this();
    }

    //    int subtypes_count = 0;
    //    for (it = li.begin(); it != li.end(); ++it) {
    //        Type &v(*it->type);
    //        if(v.isCompound()) {
    //            subtypes_count += ((CompoundType &)v).getNumTypes();
    //        }
    //        else if(v.isUnion()) {
    //            subtypes_count += ((UnionType &)v).getNumTypes();
    //        }
    //        else
    //            subtypes_count+=1;
    //    }
    //    if(subtypes_count>9) {
    //        qDebug() << getCtype();
    //        qDebug() << other->getCtype();
    //        qDebug() << "*****";
    //    }

    // Match 'other' agains all fields of 'this' UnionType
    // if a field is found that requires no change to 'meet', this type is returned unchanged
    // if a new meetWith result is 'better' given simplistic type description length heuristic measure
    // then the meetWith result, and this types field iterator are stored.

    int bestMeetScore = INT_MAX;
    UnionEntrySet::const_iterator bestElem = li.end();

    for (auto it = li.begin(); it != li.end(); ++it) {
        SharedType v = it->type;

        if (!v->isCompatibleWith(*other)) {
            continue;
        }

        changed = false;
        SharedType meet_res = v->meetWith(other, changed, useHighestPtr);

        if (!changed) {
            // Fully compatible type alerady present in this union
            return const_cast<UnionType *>(this)->shared_from_this();
        }

        const int currentScore = meet_res->getCtype().size();

        if (currentScore < bestMeetScore) {
            // we have found a better match, store it
            bestElem      = it;
            bestMeetScore = currentScore;
        }
    }


    std::shared_ptr<UnionType> result = UnionType::get();

    for (auto it = li.begin(); it != li.end(); ++it) {
        if (it == bestElem) {
            // this is the element to be replaced
            continue;
        }

        result->addType(it->type, it->name);
    }

    UnionElement ne;

    if (bestElem != li.end()) {
        ne.name = bestElem->name;
        ne.type = bestElem->type->meetWith(other, changed, useHighestPtr); // we know this works because the types are compatible
    }
    else {
        // Other is not compatible with any of my component types. Add a new type.
        ne.name = QString("x%1").arg(++nextUnionNumber);
        ne.type = other->clone();
    }

    result->addType(ne.type, ne.name);
    changed = true;
    return result;
}


bool UnionType::isCompatible(const Type& other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        if (this == &other) { // Note: pointer comparison
            return true;      // Avoid infinite recursion
        }

        const UnionType& otherUnion = static_cast<const UnionType &>(other);

        // Unions are compatible if one is a subset of the other
        if (li.size() < otherUnion.li.size()) {
            for (const UnionElement& e : li) {
                if (!otherUnion.isCompatible(*e.type, all)) {
                    return false;
                }
            }
        }
        else {
            for (const UnionElement& e : otherUnion.li) {
                if (!isCompatible(*e.type, all)) {
                    return false;
                }
            }
        }

        return true;
    }

    // Other is not a UnionType
    for (const UnionElement& e : li) {
        if (other.isCompatibleWith(*e.type, all)) {
            return true;
        }
    }

    return false;
}


// Dereference this union. If it is a union of pointers, return a union of the dereferenced items. Else return VoidType
// (note: should probably be bottom)
SharedType UnionType::dereferenceUnion()
{
    auto ret = UnionType::get();

    UnionEntrySet::iterator it;

    for (it = li.begin(); it != li.end(); ++it) {
        SharedType elem = it->type->dereference();

        if (elem->resolvesToVoid()) {
            return elem; // Return void for the whole thing
        }

        ret->addType(elem->clone());
    }

    return ret;
}
