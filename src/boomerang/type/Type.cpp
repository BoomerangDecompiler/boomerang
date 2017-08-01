/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       type.cpp
 * \brief   Implementation of the Type class: low level type information
 ******************************************************************************/
#include "Type.h"

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/CFG.h"
#include "boomerang/db/Proc.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/TypeVal.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Util.h"

#include <QtCore/QDebug>
#include <cassert>
#include <cstring>


QMap<QString, SharedType> Type::namedTypes;


bool Type::isCString() const
{
    if (!resolvesToPointer()) {
        return false;
    }

    SharedType p = as<PointerType>()->getPointsTo();

    if (p->resolvesToChar()) {
        return true;
    }

    if (!p->resolvesToArray()) {
        return false;
    }

    p = p->as<ArrayType>()->getBaseType();
    return p->resolvesToChar();
}


Type::Type(eType _id)
    : id(_id)
{
}


VoidType::VoidType()
    : Type(eVoid)
{
}


FuncType::FuncType(const std::shared_ptr<Signature>& sig)
    : Type(eFunc)
    , signature(sig)
{
}


FloatType::FloatType(int sz)
    : Type(eFloat)
    , size(sz)
{
}


std::shared_ptr<FloatType> FloatType::get(int sz)
{
    return std::make_shared<FloatType>(sz);
}


BooleanType::BooleanType()
    : Type(eBoolean)
{
}


CharType::CharType()
    : Type(eChar)
{
}


void PointerType::setPointsTo(SharedType p)
{
    if (p.get() == this) {           // Note: comparing pointers
        points_to = VoidType::get(); // Can't point to self; impossible to compare, print, etc
        LOG_WARN("Attempted to create pointer to self: %1", HostAddress(this).toString());
    }
    else {
        points_to = p;
    }
}


PointerType::PointerType(SharedType p)
    : Type(ePointer)
{
    setPointsTo(p);
}


ArrayType::ArrayType(SharedType p, unsigned _length)
    : Type(eArray)
    , BaseType(p)
    , Length(_length)
{
}


ArrayType::ArrayType(SharedType p)
    : Type(eArray)
    , BaseType(p)
    , Length(NO_BOUND)
{
}


bool ArrayType::isUnbounded() const
{
    return Length == NO_BOUND;
}


size_t ArrayType::convertLength(SharedType b) const
{
    // MVE: not sure if this is always the right thing to do
    if (Length != NO_BOUND) {
        size_t baseSize = BaseType->getSize() / 8; // Old base size (one element) in bytes

        if (baseSize == 0) {
            baseSize = 1;   // Count void as size 1
        }

        baseSize *= Length; // Old base size (length elements) in bytes
        size_t newSize = b->getSize() / 8;

        if (newSize == 0) {
            newSize = 1;
        }

        return baseSize / newSize; // Preserve same byte size for array
    }

    return NO_BOUND;
}


void ArrayType::setBaseType(SharedType b)
{
    // MVE: not sure if this is always the right thing to do
    if (Length != NO_BOUND) {
        size_t baseSize = BaseType->getSize() / 8; // Old base size (one element) in bytes

        if (baseSize == 0) {
            baseSize = 1;   // Count void as size 1
        }

        baseSize *= Length; // Old base size (length elements) in bytes
        size_t newSize = b->getSize() / 8;

        if (newSize == 0) {
            newSize = 1;
        }

        Length = baseSize / newSize; // Preserve same byte size for array
    }

    BaseType = b;
}


NamedType::NamedType(const QString& _name)
    : Type(eNamed)
    , name(_name)
{
}


CompoundType::CompoundType(bool is_generic /* = false */)
    : Type(eCompound)
    , nextGenericMemberNum(1)
    , generic(is_generic)
{
}


UnionType::UnionType()
    : Type(eUnion)
{
}


Type::~Type()
{
}


VoidType::~VoidType()
{
}


FuncType::~FuncType()
{
}


FloatType::~FloatType()
{
}


BooleanType::~BooleanType()
{
}


CharType::~CharType()
{
}


PointerType::~PointerType()
{
    // delete points_to;        // Easier for test code (which doesn't use garbage collection)
}


ArrayType::~ArrayType()
{
    // delete base_type;
}


NamedType::~NamedType()
{
}


CompoundType::~CompoundType()
{
}


UnionType::~UnionType()
{
}


std::shared_ptr<IntegerType> IntegerType::get(unsigned NumBits, int sign)
{
    return std::make_shared<IntegerType>(NumBits, sign);
}


SharedType IntegerType::clone() const
{
    return IntegerType::get(size, signedness);
}


SharedType FloatType::clone() const
{
    return FloatType::get(size);
}


SharedType BooleanType::clone() const
{
    return std::make_shared<BooleanType>();
}


SharedType CharType::clone() const
{
    return CharType::get();
}


SharedType VoidType::clone() const
{
    return VoidType::get();
}


SharedType FuncType::clone() const
{
    return FuncType::get(signature);
}


SharedType PointerType::clone() const
{
    return PointerType::get(points_to->clone());
}


SharedType ArrayType::clone() const
{
    return ArrayType::get(BaseType->clone(), Length);
}


SharedType NamedType::clone() const
{
    return NamedType::get(name);
}


SharedType CompoundType::clone() const
{
    auto t = CompoundType::get();

    for (unsigned i = 0; i < types.size(); i++) {
        t->addType(types[i]->clone(), names[i]);
    }

    return t;
}


SharedType UnionType::clone() const
{
    auto u = std::make_shared<UnionType>();

    for (UnionElement el : li) {
        u->addType(el.type, el.name);
    }

    return u;
}


SharedType SizeType::clone() const
{
    return SizeType::get(size);
}


SharedType UpperType::clone() const
{
    return std::make_shared<UpperType>(base_type->clone());
}


SharedType LowerType::clone() const
{
    return std::make_shared<LowerType>(base_type->clone());
}


size_t IntegerType::getSize() const
{
    return size;
}


size_t FloatType::getSize() const
{
    return size;
}


size_t BooleanType::getSize() const
{
    return 1;
}


size_t CharType::getSize() const
{
    return 8;
}


size_t VoidType::getSize() const
{
    return 0;
}


size_t FuncType::getSize() const
{
    return 0;                                /* always nagged me */
}


size_t PointerType::getSize() const
{
    // points_to->getSize(); // yes, it was a good idea at the time
    return STD_SIZE;
}


size_t ArrayType::getSize() const
{
    return BaseType->getSize() * Length;
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


size_t CompoundType::getSize() const
{
    int n = 0;

    for (auto& elem : types) {
        // NOTE: this assumes no padding... perhaps explicit padding will be needed
        n += elem->getSize();
    }

    return n;
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


size_t SizeType::getSize() const
{
    return size;
}


SharedType CompoundType::getType(const QString& nam)
{
    for (unsigned i = 0; i < types.size(); i++) {
        if (names[i] == nam) {
            return types[i];
        }
    }

    return nullptr;
}


// Note: n is a BIT offset
SharedType CompoundType::getTypeAtOffset(unsigned n)
{
    unsigned offset = 0;

    for (auto& elem : types) {
        if ((offset <= n) && (n < offset + elem->getSize())) {
            return elem;
        }

        offset += elem->getSize();
    }

    return nullptr;
}


// Note: n is a BIT offset
void CompoundType::setTypeAtOffset(unsigned n, SharedType ty)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < types.size(); i++) {
        if ((offset <= n) && (n < offset + types[i]->getSize())) {
            unsigned oldsz = types[i]->getSize();
            types[i] = ty;

            if (ty->getSize() < oldsz) {
                types.push_back(types[types.size() - 1]);
                names.push_back(names[names.size() - 1]);

                for (size_t _n = types.size() - 1; _n > i; _n--) {
                    types[_n] = types[_n - 1];
                    names[_n] = names[_n - 1];
                }

                types[i + 1] = SizeType::get(oldsz - ty->getSize());
                names[i + 1] = "pad";
            }

            return;
        }

        offset += types[i]->getSize();
    }
}


void CompoundType::setNameAtOffset(unsigned n, const QString& nam)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < types.size(); i++) {
        if ((offset <= n) && (n < offset + types[i]->getSize())) {
            names[i] = nam;
            return;
        }

        offset += types[i]->getSize();
    }
}


QString CompoundType::getNameAtOffset(size_t n)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < types.size(); i++) {
        // if (offset >= n && n < offset + types[i]->getSize())
        if ((offset <= n) && (n < offset + types[i]->getSize())) {
            // return getName(offset == n ? i : i - 1);
            return names[i];
        }

        offset += types[i]->getSize();
    }

    return nullptr;
}


unsigned CompoundType::getOffsetTo(unsigned n)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < n; i++) {
        offset += types[i]->getSize();
    }

    return offset;
}


unsigned CompoundType::getOffsetTo(const QString& member)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < types.size(); i++) {
        if (names[i] == member) {
            return offset;
        }

        offset += types[i]->getSize();
    }

    return (unsigned)-1;
}


unsigned CompoundType::getOffsetRemainder(unsigned n)
{
    unsigned r      = n;
    unsigned offset = 0;

    for (auto& elem : types) {
        offset += elem->getSize();

        if (offset > n) {
            break;
        }

        r -= elem->getSize();
    }

    return r;
}


SharedType Type::parseType(const char *)
{
    assert(!"Not implemented");
    return nullptr;
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


bool FloatType::operator==(const Type& other) const
{
    return other.isFloat() && (size == 0 || ((FloatType&)other).size == 0 || (size == ((FloatType&)other).size));
}


bool BooleanType::operator==(const Type& other) const
{
    return other.isBoolean();
}


bool CharType::operator==(const Type& other) const
{
    return other.isChar();
}


bool VoidType::operator==(const Type& other) const
{
    return other.isVoid();
}


bool FuncType::operator==(const Type& other) const
{
    if (!other.isFunc()) {
        return false;
    }

    // Note: some functions don't have a signature (e.g. indirect calls that have not yet been successfully analysed)
    if (signature == nullptr) {
        return ((FuncType&)other).signature == nullptr;
    }

    return *signature == *((FuncType&)other).signature;
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


bool ArrayType::operator==(const Type& other) const
{
    return other.isArray() && *BaseType == *((ArrayType&)other).BaseType && ((ArrayType&)other).Length == Length;
}


bool NamedType::operator==(const Type& other) const
{
    return other.isNamed() && (name == ((NamedType&)other).name);
}


bool CompoundType::operator==(const Type& other) const
{
    const CompoundType& cother = (CompoundType&)other;

    if (other.isCompound() && (cother.types.size() == types.size())) {
        for (unsigned i = 0; i < types.size(); i++) {
            if (!(*types[i] == *cother.types[i])) {
                return false;
            }
        }

        return true;
    }

    return false;
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


bool SizeType::operator==(const Type& other) const
{
    return other.isSize() && (size == ((SizeType&)other).size);
}


bool UpperType::operator==(const Type& other) const
{
    return other.isUpper() && *base_type == *((UpperType&)other).base_type;
}


bool LowerType::operator==(const Type& other) const
{
    return other.isLower() && *base_type == *((LowerType&)other).base_type;
}


bool Type::operator!=(const Type& other) const
{
    return !(*this == other);
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

    return (signedness < ((IntegerType&)other).signedness);
}


bool FloatType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(size < ((FloatType&)other).size);
}


bool VoidType::operator<(const Type& other) const
{
    return id < other.getId();
}


bool FuncType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    // FIXME: Need to compare signatures
    return true;
}


bool BooleanType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return true;
}


bool CharType::operator<(const Type& other) const
{
    return id < other.getId();
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


bool ArrayType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(*BaseType < *((ArrayType&)other).BaseType);
}


bool NamedType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(name < ((NamedType&)other).name);
}


bool CompoundType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return getSize() < other.getSize(); // This won't separate structs of the same size!! MVE
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


bool SizeType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(size < ((SizeType&)other).size);
}


bool UpperType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(*base_type < *((UpperType&)other).base_type);
}


bool LowerType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(*base_type < *((LowerType&)other).base_type);
}


SharedExp Type::match(SharedType pattern)
{
    if (pattern->isNamed()) {
        LOG_VERBOSE("type match: %1 to %2", this->getCtype(), pattern->getCtype());
        return Binary::get(opList, Binary::get(opEquals,
                                               Unary::get(opVar, Const::get(pattern->as<NamedType>()->getName())),
                                               std::make_shared<TypeVal>(this->clone())),
                           Terminal::get(opNil));
    }

    return nullptr;
}


SharedExp IntegerType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp FloatType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp BooleanType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp CharType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp VoidType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp FuncType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp PointerType::match(SharedType pattern)
{
    if (pattern->isPointer()) {
        LOG_VERBOSE("Got pointer match: %1 to %2", this->getCtype(), pattern->getCtype());
        return points_to->match(pattern->as<PointerType>()->getPointsTo());
    }

    return Type::match(pattern);
}


SharedExp ArrayType::match(SharedType pattern)
{
    if (pattern->isArray()) {
        return BaseType->match(pattern);
    }

    return Type::match(pattern);
}


SharedExp NamedType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp CompoundType::match(SharedType pattern)
{
    return Type::match(pattern);
}


SharedExp UnionType::match(SharedType pattern)
{
    return Type::match(pattern);
}


QString VoidType::getCtype(bool /*final*/) const
{
    return "void";
}


QString FuncType::getCtype(bool final) const
{
    if (signature == nullptr) {
        return "void (void)";
    }

    QString s;

    if (signature->getNumReturns() == 0) {
        s += "void";
    }
    else {
        s += signature->getReturnType(0)->getCtype(final);
    }

    s += " (";

    for (unsigned i = 0; i < signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += signature->getParamType(i)->getCtype(final);
    }

    s += ")";
    return s;
}


void FuncType::getReturnAndParam(QString& ret, QString& param)
{
    if (signature == nullptr) {
        ret   = "void";
        param = "(void)";
        return;
    }

    if (signature->getNumReturns() == 0) {
        ret = "void";
    }
    else {
        ret = signature->getReturnType(0)->getCtype();
    }

    QString s;
    s += " (";

    for (unsigned i = 0; i < signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += signature->getParamType(i)->getCtype();
    }

    s    += ")";
    param = s;
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


QString FloatType::getCtype(bool /*final*/) const
{
    switch (size)
    {
    case 32:
        return "float";

    case 64:
        return "double";

    default:
        return "double";
    }
}


QString BooleanType::getCtype(bool /*final*/) const
{
    return "bool";
}


QString CharType::getCtype(bool /*final*/) const
{
    return "char";
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


QString ArrayType::getCtype(bool final) const
{
    QString s = BaseType->getCtype(final);

    if (isUnbounded()) {
        return s + "[]";
    }

    return s + "[" + QString::number(Length) + "]";
}


QString NamedType::getCtype(bool /*final*/) const
{
    return name;
}


QString CompoundType::getCtype(bool final) const
{
    QString tmp("struct { ");

    for (unsigned i = 0; i < types.size(); i++) {
        tmp += types[i]->getCtype(final);

        if (names[i] != "") {
            tmp += " ";
            tmp += names[i];
        }

        tmp += "; ";
    }

    tmp += "}";
    return tmp;
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


QString SizeType::getCtype(bool /*final*/) const
{
    // Emit a comment and the size
    QString     res;
    QTextStream ost(&res);

    ost << "__size" << size;
    return res;
}


QString UpperType::getCtype(bool /*final*/) const
{
    QString     res;
    QTextStream ost(&res);

    ost << "/*upper*/(" << base_type << ")";
    return res;
}


QString LowerType::getCtype(bool /*final*/) const
{
    QString     res;
    QTextStream ost(&res);

    ost << "/*lower*/(" << base_type << ")";
    return res;
}


QString Type::prints()
{
    return getCtype(false); // For debugging
}


void Type::dump()
{
    LOG_MSG("%1", getCtype(false)); // For debugging
}


// named type accessors
void Type::addNamedType(const QString& name, SharedType type)
{
    if (namedTypes.find(name) != namedTypes.end()) {
        if (!(*type == *namedTypes[name])) {
            LOG_WARN("Redefinition of type %1", name);
            LOG_WARN(" type     = %1", type->prints());
            LOG_WARN(" previous = %1", namedTypes[name]->prints());
            namedTypes[name] = type; // WARN: was *type==*namedTypes[name], verify !
        }
    }
    else {
        // check if it is:
        // typedef int a;
        // typedef a b;
        // we then need to define b as int
        // we create clones to keep the GC happy
        if (namedTypes.find(type->getCtype()) != namedTypes.end()) {
            namedTypes[name] = namedTypes[type->getCtype()]->clone();
        }
        else {
            namedTypes[name] = type->clone();
        }
    }
}


SharedType Type::getNamedType(const QString& name)
{
    auto iter = namedTypes.find(name);

    return (iter != namedTypes.end()) ? *iter : nullptr;
}


void Type::dumpNames()
{
    for (auto it = namedTypes.begin(); it != namedTypes.end(); ++it) {
        qDebug() << it.key() << " -> " << it.value()->getCtype() << "\n";
    }
}


SharedType Type::getTempType(const QString& name)
{
    SharedType ty;
    QChar      ctype = ' ';

    if (name.size() > 3) {
        ctype = name[3];
    }

    switch (ctype.toLatin1())
    {
    // They are all int32, except for a few specials
    case 'f':
        ty = FloatType::get(32);
        break;

    case 'd':
        ty = FloatType::get(64);
        break;

    case 'F':
        ty = FloatType::get(80);
        break;

    case 'D':
        ty = FloatType::get(128);
        break;

    case 'l':
        ty = IntegerType::get(64);
        break;

    case 'h':
        ty = IntegerType::get(16);
        break;

    case 'b':
        ty = IntegerType::get(8);
        break;

    default:
        ty = IntegerType::get(32);
        break;
    }

    return ty;
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


QString FloatType::getTempName() const
{
    switch (size)
    {
    case 32:
        return "tmpf";

    case 64:
        return "tmpd";

    case 80:
        return "tmpF";

    case 128:
        return "tmpD";
    }

    return "tmp";
}


QString Type::getTempName() const
{
    return "tmp"; // what else can we do? (besides panic)
}


void Type::clearNamedTypes()
{
    namedTypes.clear();
}


int NamedType::nextAlpha = 0;
std::shared_ptr<NamedType> NamedType::getAlpha()
{
    return NamedType::get(QString("alpha%1").arg(nextAlpha++));
}


std::shared_ptr<PointerType> PointerType::newPtrAlpha()
{
    return PointerType::get(NamedType::getAlpha());
}


bool PointerType::pointsToAlpha() const
{
    // void* counts as alpha* (and may replace it soon)
    if (points_to->isVoid()) {
        return true;
    }

    if (!points_to->isNamed()) {
        return false;
    }

    return points_to->as<NamedType>()->getName().startsWith("alpha");
}


int PointerType::pointerDepth() const
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


SharedType NamedType::resolvesTo() const
{
    SharedType ty = getNamedType(name);

    if (ty && ty->isNamed()) {
        return std::static_pointer_cast<NamedType>(ty)->resolvesTo();
    }

    return ty;
}


void ArrayType::fixBaseType(SharedType b)
{
    if (BaseType == nullptr) {
        BaseType = b;
    }
    else {
        assert(BaseType->isArray());
        BaseType->as<ArrayType>()->fixBaseType(b);
    }
}


// Note: don't want to call this->resolve() for this case, since then we (probably) won't have a NamedType and the
// assert will fail
#define RESOLVES_TO_TYPE(x)                                                        \
    bool Type::resolvesTo ## x() const {                                        \
        auto ty = shared_from_this();                                            \
        if (ty->isNamed()) {                                                    \
            ty = std::static_pointer_cast<const NamedType>(ty)->resolvesTo(); }    \
        return ty && ty->is ## x();                                                \
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
RESOLVES_TO_TYPE(Upper)
RESOLVES_TO_TYPE(Lower)

bool Type::isPointerToAlpha()
{
    return isPointer() && as<PointerType>()->pointsToAlpha();
}


void Type::starPrint(QTextStream& os)
{
    os << "*" << this << "*";
}


QString Type::toString() const
{
    QString     res;
    QTextStream tgt(&res);

    tgt << *this;
    return res;
}


// A crude shortcut representation of a type
QTextStream& operator<<(QTextStream& os, const Type& t)
{
    switch (t.getId())
    {
    case eInteger:
        {
            int sg = t.as<IntegerType>()->getSignedness();
            // 'j' for either i or u, don't know which
            os << (sg == 0 ? 'j' : sg > 0 ? 'i' : 'u');
            os << t.as<IntegerType>()->getSize();
            break;
        }

    case eFloat:
        os << 'f';
        os << t.as<FloatType>()->getSize();
        break;

    case ePointer:
        os << t.as<PointerType>()->getPointsTo() << '*';
        break;

    case eSize:
        os << t.getSize();
        break;

    case eChar:
        os << 'c';
        break;

    case eVoid:
        os << 'v';
        break;

    case eBoolean:
        os << 'b';
        break;

    case eCompound:
        os << "struct";
        break;

    case eUnion:
        os << "union";
        break;

    // case eUnion:    os << t.getCtype(); break;
    case eFunc:
        os << "func";
        break;

    case eArray:
        os << '[' << t.as<ArrayType>()->getBaseType();

        if (!t.as<ArrayType>()->isUnbounded()) {
            os << ", " << t.as<ArrayType>()->getLength();
        }

        os << ']';
        break;

    case eNamed:
        os << t.as<NamedType>()->getName();
        break;

    case eUpper:
        os << "U(" << t.as<UpperType>()->getBaseType() << ')';
        break;

    case eLower:
        os << "L(" << t.as<LowerType>()->getBaseType() << ')';
        break;
    }

    return os;
}


QTextStream& operator<<(QTextStream& os, const SharedConstType& t)
{
    if (t == nullptr) {
        return os << '0';
    }

    return os << *t;
}


SharedType IntegerType::mergeWith(SharedType other) const
{
    if (*this == *other) {
        return ((IntegerType *)this)->shared_from_this();
    }

    if (!other->isInteger()) {
        return nullptr; // Can you merge with a pointer?
    }

    auto oth = std::static_pointer_cast<IntegerType>(other);
    auto ret = std::static_pointer_cast<IntegerType>(this->clone());

    if (size == 0) {
        ret->setSize(oth->getSize());
    }

    if (signedness == 0) {
        ret->setSigned(oth->getSignedness());
    }

    return ret;
}


SharedType SizeType::mergeWith(SharedType other) const
{
    SharedType ret = other->clone();

    ret->setSize(size);
    return ret;
}


SharedType UpperType::mergeWith(SharedType /*other*/) const
{
    // FIXME: TBC
    return ((UpperType *)this)->shared_from_this();
}


SharedType LowerType::mergeWith(SharedType /*other*/) const
{
    // FIXME: TBC
    return ((LowerType *)this)->shared_from_this();
}


bool CompoundType::isSuperStructOf(const SharedType& other)
{
    if (!other->isCompound()) {
        return false;
    }

    auto   otherCmp = other->as<CompoundType>();
    size_t n        = otherCmp->types.size();

    if (n > types.size()) {
        return false;
    }

    for (unsigned i = 0; i < n; i++) {
        if (otherCmp->types[i] != types[i]) {
            return false;
        }
    }

    return true;
}


bool CompoundType::isSubStructOf(SharedType other) const
{
    if (!other->isCompound()) {
        return false;
    }

    auto     otherCmp = other->as<CompoundType>();
    unsigned n        = types.size();

    if (n > otherCmp->types.size()) {
        return false;
    }

    for (unsigned i = 0; i < n; i++) {
        if (otherCmp->types[i] != types[i]) {
            return false;
        }
    }

    return true;
}


bool UnionType::findType(SharedType ty)
{
    UnionElement ue;

    ue.type = ty;
    return li.find(ue) != li.end();
}


void UpperType::setSize(size_t /*size*/)
{
    // Does this make sense?
    assert(0);
}


void LowerType::setSize(size_t /*size*/)
{
    // Does this make sense?
    assert(0);
}


SharedType Type::newIntegerLikeType(int size, int signedness)
{
    if (size == 1) {
        return BooleanType::get();
    }

    if ((size == 8) && (signedness >= 0)) {
        return CharType::get();
    }

    return IntegerType::get(size, signedness);
}


DataIntervalMap::iterator DataIntervalMap::find_it(Address addr)
{
    iterator it = dimap.upper_bound(addr); // Find the first item strictly greater than addr

    if (it == dimap.begin()) {
        return dimap.end(); // None <= this address, so no overlap possible
    }

    it--;                   // If any item overlaps, it is this one

    if ((addr >= it->first) && ((addr - it->first).value() < it->second.size)) {
        // This is the one that overlaps with addr
        return it;
    }

    return dimap.end();
}


DataIntervalEntry *DataIntervalMap::find(Address addr)
{
    iterator it = find_it(addr);

    if (it == dimap.end()) {
        return nullptr;
    }

    return &*it;
}


bool DataIntervalMap::isClear(Address addr, unsigned size)
{
    iterator it = dimap.upper_bound(addr + size - 1); // Find the first item strictly greater than address of last byte

    if (it == dimap.begin()) {
        return true; // None <= this address, so no overlap possible
    }

    it--;            // If any item overlaps, it is this one
    // Make sure the previous item ends before this one will start
    Address end;

    if (it->first + it->second.size < it->first) {
        end = Address::INVALID; // overflow
    }
    else {
        end = it->first + it->second.size;
    }

    if (end <= addr) {
        return true;
    }

    if (it->second.type->isArray() && it->second.type->as<ArrayType>()->isUnbounded()) {
        it->second.size = (addr - it->first).value();
        LOG_VERBOSE("Shrinking size of unbound array to %1 bytes", it->second.size);
        return true;
    }

    return false;
}


void DataIntervalMap::addItem(Address addr, QString name, SharedType ty, bool forced /* = false */)
{
    if (name.isNull()) {
        name = "<noname>";
    }

    DataIntervalEntry *pdie = find(addr);

    if (pdie == nullptr) {
        // Check that this new item is compatible with any items it overlaps with, and insert it
        replaceComponents(addr, name, ty, forced);
        return;
    }

    // There are two basic cases, and an error if the two data types weave
    if (pdie->first < addr) {
        // The existing entry comes first. Make sure it ends last (possibly equal last)
        if (pdie->first + pdie->second.size < addr + ty->getSize() / 8) {
            LOG_ERROR("TYPE ERROR: attempt to insert item %1 at address %2 of type %3 which weaves after %4 at %5 of type %6",
                name, addr, ty->getCtype(), pdie->second.name, pdie->first, pdie->second.type->getCtype());
            return;
        }

        enterComponent(pdie, addr, name, ty, forced);
    }
    else if (pdie->first == addr) {
        // Could go either way, depending on where the data items end
        Address endOfCurrent = pdie->first + pdie->second.size;
        Address endOfNew     = addr + ty->getSize() / 8;

        if (endOfCurrent < endOfNew) {
            replaceComponents(addr, name, ty, forced);
        }
        else if (endOfCurrent == endOfNew) {
            checkMatching(pdie, addr, name, ty, forced); // Size match; check that new type matches old
        }
        else {
            enterComponent(pdie, addr, name, ty, forced);
        }
    }
    else {
        // Old starts after new; check it also ends first
        if (pdie->first + pdie->second.size > addr + ty->getSize() / 8) {
            LOG_ERROR("TYPE ERROR: attempt to insert item %1 at %2 of type %3 which weaves before %4 at %5 of type %6",
                name, addr, ty->getCtype(), pdie->second.name, pdie->first, pdie->second.type->getCtype());
            return;
        }

        replaceComponents(addr, name, ty, forced);
    }
}


void DataIntervalMap::enterComponent(DataIntervalEntry *pdie, Address addr, const QString& /*name*/, SharedType ty,
                                     bool /*forced*/)
{
    if (pdie->second.type->resolvesToCompound()) {
        unsigned   bitOffset  = (addr - pdie->first).value() * 8;
        SharedType memberType = pdie->second.type->as<CompoundType>()->getTypeAtOffset(bitOffset);

        if (memberType->isCompatibleWith(*ty)) {
            bool ch;
            memberType = memberType->meetWith(ty, ch);
            pdie->second.type->as<CompoundType>()->setTypeAtOffset(bitOffset, memberType);
        }
        else {
            LOG_ERROR("TYPE ERROR: At address %1 type %2 is not compatible with existing structure member type %3",
                      addr, ty->getCtype(), memberType->getCtype());
        }
    }
    else if (pdie->second.type->resolvesToArray()) {
        SharedType memberType = pdie->second.type->as<ArrayType>()->getBaseType();

        if (memberType->isCompatibleWith(*ty)) {
            bool ch;
            memberType = memberType->meetWith(ty, ch);
            pdie->second.type->as<ArrayType>()->setBaseType(memberType);
        }
        else {
            LOG_ERROR("TYPE ERROR: At address %1 type %2 is not compatible with existing array member type %3",
                      addr, ty->getCtype(), memberType->getCtype());
        }
    }
    else {
        LOG_ERROR("TYPE ERROR: Existing type at address %1 is not structure or array type",
                  pdie->first);
    }
}


void DataIntervalMap::replaceComponents(Address addr, const QString& name, SharedType ty, bool /*forced*/)
{
    iterator it;
    Address  pastLast = addr + ty->getSize() / 8; // This is the byte address just past the type to be inserted

    // First check that the new entry will be compatible with everything it will overlap
    if (ty->resolvesToCompound()) {
        iterator it1 = dimap.lower_bound(addr);         // Iterator to the first overlapping item (could be end(), but
        // if so, it2 will also be end())
        iterator it2 = dimap.upper_bound(pastLast - 1); // Iterator to the first item that starts too late

        for (it = it1; it != it2; ++it) {
            unsigned bitOffset = (it->first - addr).value() * 8;

            SharedType memberType = ty->as<CompoundType>()->getTypeAtOffset(bitOffset);

            if (memberType->isCompatibleWith(*it->second.type, true)) {
                bool ch;
                qDebug() << prints();
                qDebug() << memberType->getCtype() << " " << it->second.type->getCtype();
                memberType = it->second.type->meetWith(memberType, ch);
                ty->as<CompoundType>()->setTypeAtOffset(bitOffset, memberType);
            }
            else {
                LOG_ERROR("TYPE ERROR: At address %1 struct type %2 is not compatible with existing type ",
                          addr, ty->getCtype(), it->second.type->getCtype());
                return;
            }
        }
    }
    else if (ty->resolvesToArray()) {
        SharedType memberType = ty->as<ArrayType>()->getBaseType();
        iterator   it1        = dimap.lower_bound(addr);
        iterator   it2        = dimap.upper_bound(pastLast - 1);

        for (it = it1; it != it2; ++it) {
            if (memberType->isCompatibleWith(*it->second.type, true)) {
                bool ch;
                memberType = memberType->meetWith(it->second.type, ch);
                ty->as<ArrayType>()->setBaseType(memberType);
            }
            else {
                LOG_ERROR("TYPE ERROR: At address %1 array type %2 is not compatible with existing type %3",
                          addr, ty->getCtype(), it->second.type->getCtype());
                return;
            }
        }
    }
    else {
        // Just make sure it doesn't overlap anything
        if (!isClear(addr, (ty->getSize() + 7) / 8)) {
            LOG_ERROR("TYPE ERROR: at address %1, overlapping type %2 "
                "does not resolve to compound or array", addr, ty->getCtype());
            return;
        }
    }

    // The compound or array type is compatible. Remove the items that it will overlap with
    iterator it1 = dimap.lower_bound(addr);
    iterator it2 = dimap.upper_bound(pastLast - 1);

    // Check for existing locals that need to be updated
    if (ty->resolvesToCompound() || ty->resolvesToArray()) {
        SharedExp rsp  = Location::regOf(proc->getSignature()->getStackRegister());
        auto      rsp0 = RefExp::get(rsp, proc->getCFG()->findTheImplicitAssign(rsp)); // sp{0}

        for (it = it1; it != it2; ++it) {
            // Check if there is an existing local here
            SharedExp locl = Location::memOf(Binary::get(opPlus, rsp0->clone(), Const::get(it->first.native())));
            locl->simplifyArith(); // Convert m[sp{0} + -4] to m[sp{0} - 4]
            SharedType elemTy;
            int        bitOffset = (it->first - addr).value() / 8;

            if (ty->resolvesToCompound()) {
                elemTy = ty->as<CompoundType>()->getTypeAtOffset(bitOffset);
            }
            else {
                elemTy = ty->as<ArrayType>()->getBaseType();
            }

            QString locName = proc->findLocal(locl, elemTy);

            if (!locName.isNull() && ty->resolvesToCompound()) {
                auto c = ty->as<CompoundType>();
                // want s.m where s is the new compound object and m is the member at offset bitOffset
                QString   memName = c->getNameAtOffset(bitOffset);
                SharedExp s       = Location::memOf(Binary::get(opPlus, rsp0->clone(), Const::get(addr)));
                s->simplifyArith();
                SharedExp memberExp = Binary::get(opMemberAccess, s, Const::get(memName));
                proc->mapSymbolTo(locl, memberExp);
            }
            else {
                // FIXME: to be completed
            }
        }
    }

    for (it = it1; it != it2 && it != dimap.end(); ) {
        // I believe that it is a conforming extension for map::erase() to return the iterator, but it is not portable
        // to use it. In particular, gcc considers using the return value as an error
        // The postincrement operator seems to be the definitive way to do this
        dimap.erase(it++);
    }

    DataInterval *pdi = &dimap[addr]; // Finally add the new entry
    pdi->size = ty->getBytes();
    pdi->name = name;
    pdi->type = ty;
}


void DataIntervalMap::checkMatching(DataIntervalEntry *pdie, Address addr, const QString& /*name*/, SharedType ty,
                                    bool /*forced*/)
{
    if (pdie->second.type->isCompatibleWith(*ty)) {
        // Just merge the types and exit
        bool ch = false;
        pdie->second.type = pdie->second.type->meetWith(ty, ch);
        return;
    }

    LOG_MSG("TYPE DIFFERENCE (could be OK): At address %1 existing type %2 but added type %3",
            addr, pdie->second.type->getCtype(), ty->getCtype());
}


void DataIntervalMap::deleteItem(Address addr)
{
    iterator it = dimap.find(addr);

    if (it == dimap.end()) {
        return;
    }

    dimap.erase(it);
}


void DataIntervalMap::dump()
{
    LOG_MSG(prints());
}


char *DataIntervalMap::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);
    iterator    it;

    for (it = dimap.begin(); it != dimap.end(); ++it) {
        ost << it->first << "-" << it->first + it->second.type->getBytes() << " " << it->second.name << " " << it->second.type->getCtype()
            << "\n";
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


ComplexTypeCompList& Type::compForAddress(Address addr, DataIntervalMap& dim)
{
    DataIntervalEntry   *pdie = dim.find(addr);
    ComplexTypeCompList *res  = new ComplexTypeCompList;

    if (pdie == nullptr) {
        return *res;
    }

    Address    startCurrent = pdie->first;
    SharedType curType      = pdie->second.type;

    while (startCurrent < addr) {
        size_t bitOffset = (addr - startCurrent).value() * 8;

        if (curType->isCompound()) {
            auto     compCurType = curType->as<CompoundType>();
            unsigned rem         = compCurType->getOffsetRemainder(bitOffset);
            startCurrent = addr - (rem / 8);
            ComplexTypeComp ctc;
            ctc.isArray      = false;
            ctc.u.memberName = compCurType->getNameAtOffset(bitOffset);
            res->push_back(ctc);
            curType = compCurType->getTypeAtOffset(bitOffset);
        }
        else if (curType->isArray()) {
            curType = curType->as<ArrayType>()->getBaseType();
            unsigned baseSize = curType->getSize();
            unsigned index    = bitOffset / baseSize;
            startCurrent += index * baseSize / 8;
            ComplexTypeComp ctc;
            ctc.isArray = true;
            ctc.u.index = index;
            res->push_back(ctc);
        }
        else {
            LOG_ERROR("TYPE ERROR: no struct or array at byte address %1", addr);
            return *res;
        }
    }

    return *res;
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


void CompoundType::updateGenericMember(int off, SharedType ty, bool& ch)
{
    assert(generic);
    int        bit_offset   = off * 8;
    SharedType existingType = getTypeAtOffset(bit_offset);

    if (existingType) {
        existingType = existingType->meetWith(ty, ch);
        setTypeAtOffset(bit_offset, existingType);
    }
    else {
        QString nam = QString("member") + QString::number(nextGenericMemberNum++);
        setTypeAtOffset(bit_offset, ty);
        setNameAtOffset(bit_offset, nam);
    }
}
