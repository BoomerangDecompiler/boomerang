#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CompoundType.h"

#include "boomerang/ssl/type/SizeType.h"


CompoundType::CompoundType()
    : Type(TypeClass::Compound)
{
}


CompoundType::~CompoundType()
{
}


SharedType CompoundType::clone() const
{
    auto t = CompoundType::get();

    for (unsigned i = 0; i < m_types.size(); i++) {
        t->addMember(m_types[i]->clone(), m_names[i]);
    }

    return t;
}


Type::Size CompoundType::getSize() const
{
    int n = 0;

    for (auto &elem : m_types) {
        // NOTE: this assumes no padding... perhaps explicit padding will be needed
        n += elem->getSize();
    }

    return n;
}


bool CompoundType::isSuperStructOf(const SharedType &other) const
{
    if (!other->isCompound()) {
        return false;
    }

    auto otherCmp = other->as<CompoundType>();
    size_t n      = otherCmp->m_types.size();

    if (n > m_types.size()) {
        return false;
    }

    for (unsigned i = 0; i < n; i++) {
        if (otherCmp->m_types[i] != m_types[i]) {
            return false;
        }
    }

    return true;
}


bool CompoundType::isSubStructOf(const SharedType &other) const
{
    if (!other->isCompound()) {
        return false;
    }

    auto otherCmp = other->as<CompoundType>();
    unsigned n    = m_types.size();

    if (n > otherCmp->m_types.size()) {
        return false;
    }

    for (unsigned i = 0; i < n; i++) {
        if (otherCmp->m_types[i] != m_types[i]) {
            return false;
        }
    }

    return true;
}


SharedType CompoundType::getMemberTypeByName(const QString &name)
{
    for (unsigned i = 0; i < m_types.size(); i++) {
        if (m_names[i] == name) {
            return m_types[i];
        }
    }

    return nullptr;
}


SharedType CompoundType::getMemberTypeByOffset(unsigned bitOffset)
{
    unsigned offset = 0;

    for (auto &elem : m_types) {
        if ((offset <= bitOffset) && (bitOffset < offset + elem->getSize())) {
            return elem;
        }

        offset += elem->getSize();
    }

    return nullptr;
}


void CompoundType::setMemberTypeByOffset(unsigned bitOffset, SharedType ty)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < m_types.size(); i++) {
        if ((offset <= bitOffset) && (bitOffset < offset + m_types[i]->getSize())) {
            unsigned oldsz = m_types[i]->getSize();
            m_types[i]     = ty;

            if (ty->getSize() < oldsz) {
                m_types.push_back(m_types[m_types.size() - 1]);
                m_names.push_back(m_names[m_names.size() - 1]);

                for (size_t _n = m_types.size() - 1; _n > i; _n--) {
                    m_types[_n] = m_types[_n - 1];
                    m_names[_n] = m_names[_n - 1];
                }

                m_types[i + 1] = SizeType::get(oldsz - ty->getSize());
                m_names[i + 1] = "pad";
            }

            return;
        }

        offset += m_types[i]->getSize();
    }
}


void CompoundType::setMemberNameByOffset(unsigned n, const QString &name)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < m_types.size(); i++) {
        if ((offset <= n) && (n < offset + m_types[i]->getSize())) {
            m_names[i] = name;
            return;
        }

        offset += m_types[i]->getSize();
    }
}


QString CompoundType::getMemberNameByOffset(size_t n)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < m_types.size(); i++) {
        // if (offset >= n && n < offset + types[i]->getSize())
        if ((offset <= n) && (n < offset + m_types[i]->getSize())) {
            // return getName(offset == n ? i : i - 1);
            return m_names[i];
        }

        offset += m_types[i]->getSize();
    }

    return nullptr;
}


unsigned CompoundType::getMemberOffsetByIdx(int n)
{
    unsigned offset = 0;

    for (int i = 0; i < n; i++) {
        offset += m_types[i]->getSize();
    }

    return offset;
}


unsigned CompoundType::getMemberOffsetByName(const QString &member)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < m_types.size(); i++) {
        if (m_names[i] == member) {
            return offset;
        }

        offset += m_types[i]->getSize();
    }

    return static_cast<unsigned int>(-1);
}


unsigned CompoundType::getOffsetRemainder(unsigned n)
{
    unsigned r      = n;
    unsigned offset = 0;

    for (auto &elem : m_types) {
        offset += elem->getSize();

        if (offset > n) {
            break;
        }

        r -= elem->getSize();
    }

    return r;
}


bool CompoundType::operator==(const Type &other) const
{
    if (getId() != other.getId()) {
        return false;
    }
    else if (getSize() != other.getSize()) {
        return false;
    }

    const CompoundType &otherCompound = static_cast<const CompoundType &>(other);
    if (getNumMembers() != otherCompound.getNumMembers()) {
        return false;
    }

    for (int i = 0; i < getNumMembers(); ++i) {
        if (*m_types[i] != *otherCompound.m_types[i]) {
            return false;
        }
    }

    return true;
}


bool CompoundType::operator<(const Type &other) const
{
    if (getId() != other.getId()) {
        return getId() < other.getId();
    }

    if (getSize() != other.getSize()) {
        return getSize() < other.getSize();
    }

    const CompoundType &otherCompound = static_cast<const CompoundType &>(other);
    if (getNumMembers() != otherCompound.getNumMembers()) {
        return getNumMembers() < otherCompound.getNumMembers();
    }

    for (int i = 0; i < getNumMembers(); ++i) {
        const bool less = *m_types[i] < *otherCompound.m_types[i];
        if (!less) {
            return false;
        }
    }

    return true;
}


QString CompoundType::getCtype(bool final) const
{
    QString tmp("struct { ");

    for (unsigned i = 0; i < m_types.size(); i++) {
        tmp += m_types[i]->getCtype(final);

        if (m_names[i] != "") {
            tmp += " ";
            tmp += m_names[i];
        }

        tmp += "; ";
    }

    tmp += "}";
    return tmp;
}


void CompoundType::addMember(SharedType memberType, const QString &memberName)
{
    // check if it is a user defined type (typedef)
    SharedType existingType = getNamedType(memberType->getCtype());

    if (existingType != nullptr) {
        memberType = existingType;
    }

    m_types.push_back(memberType);
    m_names.push_back(memberName);
}


SharedType CompoundType::getMemberTypeByIdx(int idx)
{
    assert(idx < getNumMembers());
    return m_types[idx];
}


QString CompoundType::getMemberNameByIdx(int idx)
{
    assert(idx < getNumMembers());
    return m_names[idx];
}


SharedType CompoundType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<CompoundType *>(this)->shared_from_this();
    }

    if (!other->resolvesToCompound()) {
        if (m_types[0]->isCompatibleWith(*other)) {
            // struct meet first element = struct
            return const_cast<CompoundType *>(this)->shared_from_this();
        }

        return createUnion(other, changed, useHighestPtr);
    }

    std::shared_ptr<CompoundType> otherCmp = other->as<CompoundType>();

    if (*this == *other) {
        return const_cast<CompoundType *>(this)->shared_from_this();
    }
    else if (otherCmp->isSuperStructOf(const_cast<CompoundType *>(this)->shared_from_this())) {
        // The other structure has a superset of my struct's offsets. Preserve the names etc of the
        // bigger struct.
        changed = true;
        return other;
    }
    else if (isSubStructOf(otherCmp)) {
        // This is a superstruct of other
        changed = true;
        return const_cast<CompoundType *>(this)->shared_from_this();
    }

    // Not compatible structs. Create a union of both complete structs.
    // NOTE: may be possible to take advantage of some overlaps of the two structures some day.
    return createUnion(other, changed, useHighestPtr);
}


bool CompoundType::isCompatible(const Type &other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (!other.resolvesToCompound()) {
        // Used to always return false here. But in fact, a struct is compatible with its first
        // member (if all is false)
        return !all && m_types[0]->isCompatibleWith(other);
    }

    auto otherComp = other.as<CompoundType>();
    size_t n       = otherComp->getNumMembers();

    if (n != m_types.size()) {
        return false; // Is a subcompound compatible with a supercompound?
    }

    for (size_t i = 0; i < n; i++) {
        if (!m_types[i]->isCompatibleWith(*otherComp->m_types[i])) {
            return false;
        }
    }

    return true;
}
