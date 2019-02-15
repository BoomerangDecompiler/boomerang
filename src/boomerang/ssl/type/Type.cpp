#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Type.h"

#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/NamedType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/type/DataIntervalMap.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"

#include <QMap>

#include <cassert>
#include <cstring>


/// For NamedType
static QMap<QString, SharedType> g_namedTypes;


Type::Type(TypeClass _class)
    : m_id(_class)
{
}


Type::~Type()
{
}


void Type::setSize(Type::Size)
{
    assert(false); /* Redefined in subclasses. */
}


bool Type::isCString() const
{
    return (resolvesToPointer() && this->as<PointerType>()->getPointsTo()->resolvesToChar()) ||
           (resolvesToArray() && this->as<ArrayType>()->getBaseType()->resolvesToChar());
}


bool Type::operator!=(const Type &other) const
{
    return !(*this == other);
}


void Type::addNamedType(const QString &name, SharedType type)
{
    if (g_namedTypes.find(name) != g_namedTypes.end()) {
        if (!(*type == *g_namedTypes[name])) {
            LOG_WARN("Redefinition of type %1", name);
            LOG_WARN(" type     = %1", type->getCtype());
            LOG_WARN(" previous = %1", g_namedTypes[name]->getCtype());
            g_namedTypes[name] = type; // WARN: was *type==*namedTypes[name], verify !
        }
    }
    else {
        // check if it is:
        // typedef int a;
        // typedef a b;
        // we then need to define b as int
        // we create clones to keep the GC happy
        if (g_namedTypes.find(type->getCtype()) != g_namedTypes.end()) {
            g_namedTypes[name] = g_namedTypes[type->getCtype()]->clone();
        }
        else {
            g_namedTypes[name] = type->clone();
        }
    }
}


SharedType Type::getNamedType(const QString &name)
{
    auto iter = g_namedTypes.find(name);

    return (iter != g_namedTypes.end()) ? *iter : nullptr;
}


void Type::clearNamedTypes()
{
    g_namedTypes.clear();
}


// Note: don't want to call this->resolve() for this case, since then we (probably) won't have a
// NamedType and the assert will fail
#define RESOLVES_TO_TYPE(x)                                                                        \
    bool Type::resolvesTo##x() const                                                               \
    {                                                                                              \
        if (!isNamed()) {                                                                          \
            return this->is##x();                                                                  \
        }                                                                                          \
        else {                                                                                     \
            SharedType ty = this->as<NamedType>()->resolvesTo();                                   \
            return ty && ty->is##x();                                                              \
        }                                                                                          \
    }

RESOLVES_TO_TYPE(Void)
RESOLVES_TO_TYPE(Func)
RESOLVES_TO_TYPE(Boolean)
RESOLVES_TO_TYPE(Char)
RESOLVES_TO_TYPE(Integer)
RESOLVES_TO_TYPE(Float)
RESOLVES_TO_TYPE(Pointer)
RESOLVES_TO_TYPE(Array)
RESOLVES_TO_TYPE(Compound)
RESOLVES_TO_TYPE(Union)
RESOLVES_TO_TYPE(Size)


bool Type::resolvesToFuncPtr() const
{
    return resolvesToPointer() && as<PointerType>()->getPointsTo()->resolvesToFunc();
}


SharedType Type::resolveNamedType()
{
    if (isNamed()) {
        return this->as<NamedType>()->resolvesTo();
    }
    else {
        return shared_from_this();
    }
}


SharedConstType Type::resolveNamedType() const
{
    if (isNamed()) {
        return this->as<NamedType>()->resolvesTo();
    }
    else {
        return shared_from_this();
    }
}


QString Type::toString() const
{
    QString res;
    OStream tgt(&res);

    tgt << *this;
    return res;
}


// A crude shortcut representation of a type
OStream &operator<<(OStream &os, const Type &type)
{
    switch (type.getId()) {
    case TypeClass::Integer: {
        int sg = (int)type.as<IntegerType>()->getSign();
        // 'j' for either i or u, don't know which
        os << (sg == 0 ? 'j' : sg > 0 ? 'i' : 'u');
        os << type.as<IntegerType>()->getSize();
        break;
    }

    case TypeClass::Float:
        os << 'f';
        os << type.as<FloatType>()->getSize();
        break;

    case TypeClass::Pointer: os << type.as<PointerType>()->getPointsTo() << '*'; break;
    case TypeClass::Size: os << type.getSize(); break;
    case TypeClass::Char: os << 'c'; break;
    case TypeClass::Void: os << 'v'; break;
    case TypeClass::Boolean: os << 'b'; break;
    case TypeClass::Compound: os << "struct"; break;
    case TypeClass::Union: os << "union"; break;
    case TypeClass::Func: os << "func"; break;
    case TypeClass::Named: os << type.as<NamedType>()->getName(); break;
    case TypeClass::Array:
        os << '[' << type.as<ArrayType>()->getBaseType();

        if (!type.as<ArrayType>()->isUnbounded()) {
            os << ", " << type.as<ArrayType>()->getLength();
        }

        os << ']';
        break;
    }

    return os;
}


OStream &operator<<(OStream &os, const SharedConstType &t)
{
    if (t == nullptr) {
        return os << '0';
    }

    return os << *t;
}


SharedType Type::newIntegerLikeType(Size size, Sign signedness)
{
    if (size == 1) {
        return BooleanType::get();
    }
    else if (size == 8 && signedness >= Sign::Unknown) {
        return CharType::get();
    }

    return IntegerType::get(size, signedness);
}


SharedType Type::createUnion(SharedType other, bool &changed, bool useHighestPtr) const
{
    // `this' should not be a UnionType
    assert(!resolvesToUnion());

    // Put all the hard union logic in one place
    if (other->resolvesToUnion()) {
        SharedType result = const_cast<Type *>(this)->shared_from_this();
        result            = other->meetWith(result, changed, useHighestPtr)->clone();
        changed           = true;
        return result;
    }

    // Check for anytype meet compound with anytype as first element
    if (other->resolvesToCompound()) {
        auto otherComp       = other->as<CompoundType>();
        SharedType firstType = otherComp->getMemberTypeByIdx(0);

        if (firstType->isCompatibleWith(*this)) {
            // struct meet first element = struct
            return other->clone();
        }
    }

    // Check for anytype meet array of anytype
    if (other->resolvesToArray()) {
        auto otherArr     = other->as<ArrayType>();
        SharedType elemTy = otherArr->getBaseType();

        if (elemTy->isCompatibleWith(*this)) {
            // x meet array[x] == array[x]
            changed = true; // since 'this' type is not an array, but the returned type is
            return other->clone();
        }
    }

    changed = true;
    return std::make_shared<UnionType>(
        std::initializer_list<SharedType>{ this->clone(), other->clone() });
}


bool Type::isCompatibleWith(const Type &other, bool all /* = false */) const
{
    // Note: to prevent infinite recursion, CompoundType, ArrayType, and UnionType
    // implement this function as a delegation to isCompatible()
    if (other.resolvesToCompound() || other.resolvesToArray() || other.resolvesToUnion()) {
        return other.isCompatible(*this, all);
    }

    return isCompatible(other, all);
}


bool Type::isSubTypeOrEqual(SharedType other) const
{
    if (resolvesToVoid() || *this == *other) {
        return true;
    }
    else if (this->resolvesToCompound() && other->resolvesToCompound()) {
        return this->as<CompoundType>()->isSubStructOf(other);
    }

    // Not really sure here
    return false;
}
