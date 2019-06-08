#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PointerType.h"

#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/log/Log.h"


PointerType::PointerType(SharedType p)
    : Type(TypeClass::Pointer)
{
    setPointsTo(p);
}


PointerType::~PointerType()
{
}

std::shared_ptr<PointerType> PointerType::get(SharedType pointsTo)
{
    return std::make_shared<PointerType>(pointsTo);
}


void PointerType::setPointsTo(SharedType p)
{
    assert(p != nullptr);

    // Can't point to self; impossible to compare, print, etc
    if (p.get() == this) {
        LOG_ERROR("Attempted to create pointer to self: %1", HostAddress(this).toString());
        m_pointsTo = VoidType::get();
        return;
    }

    m_pointsTo = p;
}


SharedType PointerType::clone() const
{
    return PointerType::get(m_pointsTo->clone());
}


Type::Size PointerType::getSize() const
{
    return STD_SIZE;
}


void PointerType::setSize(Type::Size sz)
{
    Q_UNUSED(sz);
    assert(sz == STD_SIZE);
}


bool PointerType::operator==(const Type &other) const
{
    if (!other.isPointer()) {
        return false;
    }

    SharedType myPointsTo    = m_pointsTo;
    SharedType otherPointsTo = static_cast<const PointerType &>(other).m_pointsTo;

    int pointerCompareNest = 0;

    while (pointerCompareNest++ < 20) {
        if (myPointsTo->isPointer() != otherPointsTo->isPointer()) {
            return false;
        }
        else if (myPointsTo->isPointer()) { // both are pointers
            myPointsTo    = myPointsTo->as<PointerType>()->getPointsTo();
            otherPointsTo = otherPointsTo->as<PointerType>()->getPointsTo();
        }
        else { // neither are pointers
            return *myPointsTo == *otherPointsTo;
        }
    }

    LOG_VERBOSE("PointerType operator== nesting depth exceeded!");
    return true;
}


bool PointerType::operator<(const Type &other) const
{
    if (m_id != other.getId()) {
        return m_id < other.getId();
    }

    return *m_pointsTo < *static_cast<const PointerType &>(other).m_pointsTo;
}


bool PointerType::isVoidPointer() const
{
    return m_pointsTo->isVoid();
}


int PointerType::getPointerDepth() const
{
    int d   = 1;
    auto pt = m_pointsTo;

    while (pt->isPointer()) {
        pt = pt->as<PointerType>()->getPointsTo();
        d++;
    }

    return d;
}


SharedType PointerType::getFinalPointsTo() const
{
    SharedType pt = m_pointsTo;

    while (pt->isPointer()) {
        pt = pt->as<PointerType>()->getPointsTo();
    }

    return pt;
}


QString PointerType::getCtype(bool final) const
{
    QString s = m_pointsTo->getCtype(final);

    if (m_pointsTo->isPointer()) {
        s += "*";
    }
    else {
        s += " *";
    }

    return s; // memory..
}


SharedType PointerType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return std::const_pointer_cast<PointerType>(this->as<PointerType>());
    }

    if (other->resolvesToSize() && (other->as<SizeType>()->getSize() == STD_SIZE)) {
        return std::const_pointer_cast<PointerType>(this->as<PointerType>());
    }

    if (!other->resolvesToPointer()) {
        // Would be good to understand class hierarchies, so we know if a* is the same as b* when b
        // is a subclass of a
        return createUnion(other, changed, useHighestPtr);
    }

    auto otherPtr = other->as<PointerType>();

    if (isVoidPointer() && !otherPtr->isVoidPointer()) {
        changed = true;

        // Can't point to self; impossible to compare, print, etc
        if (otherPtr->getPointsTo() == shared_from_this()) {
            return VoidType::get(); // TODO: pointer to void at least ?
        }

        return PointerType::get(otherPtr->getPointsTo());
    }

    // We have a meeting of two pointers.
    SharedType thisBase  = m_pointsTo;
    SharedType otherBase = otherPtr->m_pointsTo;

    if (useHighestPtr) {
        // We want the greatest type of thisBase and otherBase
        if (thisBase->isSubTypeOrEqual(otherBase)) {
            return other->clone();
        }

        if (otherBase->isSubTypeOrEqual(thisBase)) {
            return std::const_pointer_cast<PointerType>(this->as<PointerType>());
        }

        // There may be another type that is a superset of this and other; for now return void*
        return PointerType::get(VoidType::get());
    }

    // See if the base types will meet
    if (otherBase->resolvesToPointer()) {
        if (thisBase->resolvesToPointer() &&
            (thisBase->as<PointerType>()->getPointsTo() == thisBase)) {
            LOG_VERBOSE("HACK! BAD POINTER 1");
        }

        if (otherBase->resolvesToPointer() &&
            (otherBase->as<PointerType>()->getPointsTo() == otherBase)) {
            LOG_VERBOSE("HACK! BAD POINTER 2");
        }

        if (thisBase == otherBase || *thisBase == *otherBase) {
            return std::const_pointer_cast<PointerType>(this->as<PointerType>());
        }

        if (getPointerDepth() == otherPtr->getPointerDepth()) {
            SharedType finalType = getFinalPointsTo();

            if (finalType->resolvesToVoid()) {
                return other->clone();
            }

            SharedType otherFinalType = otherPtr->getFinalPointsTo();

            if (otherFinalType->resolvesToVoid() || *finalType == *otherFinalType) {
                return std::const_pointer_cast<PointerType>(this->as<PointerType>());
            }
        }
    }

    if (thisBase->isCompatibleWith(*otherBase)) {
        // meet recursively if the types are compatible
        return PointerType::get(m_pointsTo->meetWith(otherBase, changed, useHighestPtr));
    }

    // The bases did not meet successfully. Union the pointers.
    return createUnion(other, changed, useHighestPtr);
}


bool PointerType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == STD_SIZE) {
        return true;
    }

    if (!other.resolvesToPointer()) {
        return false;
    }

    return m_pointsTo->isCompatibleWith(*other.as<PointerType>()->m_pointsTo);
}
