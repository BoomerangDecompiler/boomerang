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


#include "boomerang/type/type/SizeType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/Log.h"


PointerType::PointerType(SharedType p)
    : Type(ePointer)
{
    setPointsTo(p);
}


PointerType::~PointerType()
{
}


void PointerType::setPointsTo(SharedType p)
{
    // Can't point to self; impossible to compare, print, etc
    if (p.get() == this) {
        LOG_ERROR("Attempted to create pointer to self: %1", HostAddress(this).toString());
        points_to = VoidType::get();
        return;
    }

    points_to = p;
}


SharedType PointerType::clone() const
{
    return PointerType::get(points_to->clone());
}


size_t PointerType::getSize() const
{
    // points_to->getSize(); // yes, it was a good idea at the time
    return STD_SIZE;
}


static int pointerCompareNest = 0;

bool PointerType::operator==(const Type& other) const
{
    //    return other.isPointer() && (*points_to == *((PointerType&)other).points_to);
    if (!other.isPointer()) {
        return false;
    }

    if (++pointerCompareNest >= 20) {
        LOG_WARN("PointerType operator== nesting depth exceeded!");
        return true;
    }

    bool ret = (*points_to == *((PointerType&)other).points_to);
    pointerCompareNest--;
    return ret;
}


bool PointerType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(*points_to < *((PointerType&)other).points_to);
}


bool PointerType::isVoidPointer() const
{
    return points_to->isVoid();
}


int PointerType::getPointerDepth() const
{
    int  d  = 1;
    auto pt = points_to;

    while (pt->isPointer()) {
        pt = pt->as<PointerType>()->getPointsTo();
        d++;
    }

    return d;
}


SharedType PointerType::getFinalPointsTo() const
{
    SharedType pt = points_to;

    while (pt->isPointer()) {
        pt = pt->as<PointerType>()->getPointsTo();
    }

    return pt;
}


QString PointerType::getCtype(bool final) const
{
    QString s = points_to->getCtype(final);

    if (points_to->isPointer()) {
        s += "*";
    }
    else {
        s += " *";
    }

    return s; // memory..
}


SharedType PointerType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((PointerType *)this)->shared_from_this();
    }

    if (other->resolvesToSize() && (other->as<SizeType>()->getSize() == STD_SIZE)) {
        return ((PointerType *)this)->shared_from_this();
    }

    if (!other->resolvesToPointer()) {
        // Would be good to understand class hierarchies, so we know if a* is the same as b* when b is a subclass of a
        return createUnion(other, ch, bHighestPtr);
    }

    auto otherPtr = other->as<PointerType>();

    if (isVoidPointer() && !otherPtr->isVoidPointer()) {
        ch = true;

        // Can't point to self; impossible to compare, print, etc
        if (otherPtr->getPointsTo() == shared_from_this()) {
            return VoidType::get(); // TODO: pointer to void at least ?
        }

        return PointerType::get(otherPtr->getPointsTo());
    }

    // We have a meeting of two pointers.
    SharedType thisBase  = points_to;
    SharedType otherBase = otherPtr->points_to;

    if (bHighestPtr) {
        // We want the greatest type of thisBase and otherBase
        if (thisBase->isSubTypeOrEqual(otherBase)) {
            return other->clone();
        }

        if (otherBase->isSubTypeOrEqual(thisBase)) {
            return ((PointerType *)this)->shared_from_this();
        }

        // There may be another type that is a superset of this and other; for now return void*
        return PointerType::get(VoidType::get());
    }

    // See if the base types will meet
    if (otherBase->resolvesToPointer()) {
        if (thisBase->resolvesToPointer() && (thisBase->as<PointerType>()->getPointsTo() == thisBase)) {
            LOG_VERBOSE("HACK! BAD POINTER 1");
        }

        if (otherBase->resolvesToPointer() && (otherBase->as<PointerType>()->getPointsTo() == otherBase)) {
            LOG_VERBOSE("HACK! BAD POINTER 2");
        }

        if (thisBase == otherBase) {                          // Note: compare pointers
            return ((PointerType *)this)->shared_from_this(); // Crude attempt to prevent stack overflow
        }

        if (*thisBase == *otherBase) {
            return ((PointerType *)this)->shared_from_this();
        }

        if (getPointerDepth() == otherPtr->getPointerDepth()) {
            SharedType fType = getFinalPointsTo();

            if (fType->resolvesToVoid()) {
                return other->clone();
            }

            SharedType ofType = otherPtr->getFinalPointsTo();

            if (ofType->resolvesToVoid()) {
                return ((PointerType *)this)->shared_from_this();
            }

            if (*fType == *ofType) {
                return ((PointerType *)this)->shared_from_this();
            }
        }
    }

    if (thisBase->isCompatibleWith(*otherBase)) {
        // meet recursively if the types are compatible
        return PointerType::get(points_to->meetWith(otherBase, ch, bHighestPtr));
    }

    // The bases did not meet successfully. Union the pointers.
    return createUnion(other, ch, bHighestPtr);
}


bool PointerType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == STD_SIZE)) {
        return true;
    }

    if (!other.resolvesToPointer()) {
        return false;
    }

    return points_to->isCompatibleWith(*other.as<PointerType>()->points_to);
}
