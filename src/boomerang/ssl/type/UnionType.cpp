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


bool lessType::operator()(const SharedConstType &lhs, const SharedConstType &rhs) const
{
    return *lhs < *rhs;
}


UnionType::UnionType()
    : Type(TypeClass::Union)
{
}


UnionType::UnionType(const std::initializer_list<SharedType> members)
    : Type(TypeClass::Union)
{
    for (SharedType member : members) {
        addType(member, "");
    }
}


UnionType::UnionType(const std::initializer_list<UnionType::Member> members)
    : Type(TypeClass::Union)
{
    for (const Member &member : members) {
        addType(member.first, member.second);
    }
}


UnionType::~UnionType()
{
}

std::shared_ptr<UnionType> UnionType::get()
{
    return std::make_shared<UnionType>();
}


std::shared_ptr<UnionType> UnionType::get(const std::initializer_list<SharedType> members)
{
    return std::make_shared<UnionType>(members);
}


std::shared_ptr<UnionType> UnionType::get(const std::initializer_list<Member> members)
{
    return std::make_shared<UnionType>(members);
}


SharedType UnionType::clone() const
{
    std::shared_ptr<UnionType> u = std::make_shared<UnionType>();

    for (auto &[ty, name] : m_entries) {
        u->addType(ty, name);
    }

    return u;
}


Type::Size UnionType::getSize() const
{
    Size max = 0;

    for (auto &[ty, name] : m_entries) {
        Q_UNUSED(name);
        max = std::max(max, ty->getSize());
    }

    return std::max(max, (Size)1);
}


size_t UnionType::getNumTypes() const
{
    return m_entries.size();
}


bool UnionType::operator==(const Type &other) const
{
    if (!other.isUnion()) {
        return false;
    }

    const UnionType &uother = static_cast<const UnionType &>(other);

    if (uother.getNumTypes() != getNumTypes()) {
        return false;
    }

    for (const auto &[ty, name] : m_entries) {
        Q_UNUSED(name);
        if (uother.m_entries.find(ty) == uother.m_entries.end()) {
            return false;
        }
    }

    return true;
}


bool UnionType::operator<(const Type &other) const
{
    if (m_id != other.getId()) {
        return m_id < other.getId();
    }

    const UnionType &otherUnion = static_cast<const UnionType &>(other);
    if (getNumTypes() != otherUnion.getNumTypes()) {
        return getNumTypes() < otherUnion.getNumTypes();
    }

    for (auto myIt = m_entries.begin(), otherIt = otherUnion.m_entries.begin();
         myIt != m_entries.end(); ++myIt, ++otherIt) {
        // treat unions with same types but different names as equal
        if (*myIt->first != *otherIt->first) {
            return *myIt->first < *otherIt->first;
        }
    }

    return false; // equal
}


bool UnionType::hasType(SharedType ty)
{
    return m_entries.find(ty) != m_entries.end();
}


void UnionType::addType(SharedType newType, const QString &name)
{
    assert(newType != nullptr);

    if (newType->resolvesToVoid()) {
        return;
    }
    if (newType->resolvesToUnion()) {
        auto unionTy = newType->as<UnionType>();
        // Note: need to check for name clashes eventually
        m_entries.insert(unionTy->m_entries.begin(), unionTy->m_entries.end());
    }
    else {
        if (newType->resolvesToSize()) {
            for (auto &[ty, nm] : m_entries) {
                Q_UNUSED(nm);
                if (ty->getSize() == newType->getSize()) {
                    return;
                }
            }
        }

        if (newType->isPointer() && newType->as<PointerType>()->getPointsTo()->resolvesToUnion()) {
            // Explicitly disallow meeting unions and pointers to unions.
            // This can happen in binaries containing code similar to this (-> exception handling):
            //   1  x1 = ...
            //   2  x2 = phi(x1, x3)
            //   3  x3 = m[x2]
            //   4  goto 2
            // Repeatedly analyzing types for the above snippet will just result in nested
            // union types and pointers to union types ad infinitum.
            LOG_VERBOSE("Attempting to meet union with pointer to union - Not supported!");
            newType = PointerType::get(VoidType::get());
        }

        m_entries.insert({ newType, name });
        // TODO: update name if not inserted because of type clash
    }
}


QString UnionType::getCtype(bool final) const
{
    QString tmp("union { ");

    for (const auto &[ty, name] : m_entries) {
        tmp += ty->getCtype(final);

        if (name != "") {
            if (!ty->isPointer()) {
                tmp += " ";
            }

            tmp += name;
        }

        tmp += "; ";
    }

    tmp += "}";
    return tmp;
}


static int nextUnionNumber = 0;

SharedType UnionType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return this->simplify(changed);
    }

    if (other->resolvesToUnion()) {
        if (this == other.get()) {          // Note: pointer comparison
            return this->simplify(changed); // Avoid infinite recursion
        }

        std::shared_ptr<UnionType> otherUnion = other->as<UnionType>();
        SharedType result                     = this->clone();

        for (const auto &[ty, name] : otherUnion->m_entries) {
            Q_UNUSED(name);
            bool thisChanged = false;
            result           = result->meetWith(ty, thisChanged, useHighestPtr);
            changed |= thisChanged;
        }

        if (result->isUnion()) {
            result->as<UnionType>()->simplify(changed);
        }
        return result;
    }

    // Other is a non union type
    if (other->resolvesToPointer() && (other->as<PointerType>()->getPointsTo().get() == this)) {
        LOG_WARN("Attempt to union '%1' with pointer to self!", this->getCtype());
        return this->simplify(changed);
    }

    // Match 'other' agains all fields of 'this' UnionType
    // if a field is found that requires no change to 'meet', this type is returned unchanged
    // if a new meetWith result is 'better' given simplistic type description length heuristic
    // measure then the meetWith result, and this types field iterator are stored.

    int bestMeetScore                     = INT_MAX;
    UnionEntries::const_iterator bestElem = m_entries.end();

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        SharedType v = it->first;

        if (!v->isCompatibleWith(*other)) {
            continue;
        }

        bool thisChanged    = false;
        SharedType meet_res = v->meetWith(other, thisChanged, useHighestPtr);

        if (!thisChanged) {
            // Fully compatible type already present in this union
            return this->simplify(changed);
        }

        const int currentScore = meet_res->getCtype().size();

        if (currentScore < bestMeetScore) {
            // we have found a better match, store it
            bestElem      = it;
            bestMeetScore = currentScore;
        }
    }


    std::shared_ptr<UnionType> result = UnionType::get();

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        if (it == bestElem) {
            // this is the element to be replaced
            continue;
        }

        result->addType(it->first, it->second);
    }

    if (bestElem != m_entries.end()) {
        // we know this works because the types are compatible
        result->addType(bestElem->first->meetWith(other, changed, useHighestPtr), bestElem->second);
    }
    else {
        // Other is not compatible with any of my component types. Add a new type.
        result->addType(other->clone(), QString("x%1").arg(++nextUnionNumber));
    }

    changed = true;
    return result->simplify(changed);
}


bool UnionType::isCompatible(const Type &other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }
    else if (other.resolvesToUnion()) {
        if (this == &other) { // Note: pointer comparison
            return true;      // Avoid infinite recursion
        }

        const UnionType &otherUnion = static_cast<const UnionType &>(other);

        // Unions are compatible if one is a subset of the other
        if (getNumTypes() < otherUnion.getNumTypes()) {
            for (const auto &[ty, name] : m_entries) {
                Q_UNUSED(name);
                if (!otherUnion.isCompatible(*ty, all)) {
                    return false;
                }
            }
        }
        else {
            for (const auto &[ty, name] : otherUnion.m_entries) {
                Q_UNUSED(name);
                if (!isCompatible(*ty, all)) {
                    return false;
                }
            }
        }

        return true;
    }

    // Other is not a UnionType -> return true if any type is compatible
    for (const auto &[ty, name] : m_entries) {
        Q_UNUSED(name);
        if (other.isCompatibleWith(*ty, all)) {
            return true;
        }
    }

    return false;
}


SharedType UnionType::simplify(bool &changed) const
{
    if (getNumTypes() == 0) {
        changed = true;
        return VoidType::get();
    }
    else if (getNumTypes() == 1) {
        changed = true;
        return m_entries.begin()->first->clone();
    }
    else {
        return const_cast<UnionType *>(this)->shared_from_this();
    }
}


bool UnionType::isCompatibleWith(const Type &other, bool all) const
{
    return isCompatible(other, all);
}
