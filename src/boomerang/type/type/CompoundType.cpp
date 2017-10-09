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


#include "boomerang/type/type/SizeType.h"


CompoundType::CompoundType(bool is_generic /* = false */)
    : Type(eCompound)
    , m_isGeneric(is_generic)
    , m_nextGenericMemberNum(1)
{
}


CompoundType::~CompoundType()
{
}


SharedType CompoundType::clone() const
{
    auto t = CompoundType::get();

    for (unsigned i = 0; i < m_types.size(); i++) {
        t->addType(m_types[i]->clone(), m_names[i]);
    }

    return t;
}


size_t CompoundType::getSize() const
{
    int n = 0;

    for (auto& elem : m_types) {
        // NOTE: this assumes no padding... perhaps explicit padding will be needed
        n += elem->getSize();
    }

    return n;
}


bool CompoundType::isSuperStructOf(const SharedType& other)
{
    if (!other->isCompound()) {
        return false;
    }

    auto   otherCmp = other->as<CompoundType>();
    size_t n        = otherCmp->m_types.size();

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


bool CompoundType::isSubStructOf(SharedType other) const
{
    if (!other->isCompound()) {
        return false;
    }

    auto     otherCmp = other->as<CompoundType>();
    unsigned n        = m_types.size();

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


SharedType CompoundType::getType(const QString& name)
{
    for (unsigned i = 0; i < m_types.size(); i++) {
        if (m_names[i] == name) {
            return m_types[i];
        }
    }

    return nullptr;
}


SharedType CompoundType::getTypeAtOffset(unsigned bitOffset)
{
    unsigned offset = 0;

    for (auto& elem : m_types) {
        if ((offset <= bitOffset) && (bitOffset < offset + elem->getSize())) {
            return elem;
        }

        offset += elem->getSize();
    }

    return nullptr;
}


void CompoundType::setTypeAtOffset(unsigned bitOffset, SharedType ty)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < m_types.size(); i++) {
        if ((offset <= bitOffset) && (bitOffset < offset + m_types[i]->getSize())) {
            unsigned oldsz = m_types[i]->getSize();
            m_types[i] = ty;

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


void CompoundType::setNameAtOffset(unsigned n, const QString& nam)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < m_types.size(); i++) {
        if ((offset <= n) && (n < offset + m_types[i]->getSize())) {
            m_names[i] = nam;
            return;
        }

        offset += m_types[i]->getSize();
    }
}


QString CompoundType::getNameAtOffset(size_t n)
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


unsigned CompoundType::getOffsetTo(unsigned n)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < n; i++) {
        offset += m_types[i]->getSize();
    }

    return offset;
}


unsigned CompoundType::getOffsetTo(const QString& member)
{
    unsigned offset = 0;

    for (unsigned i = 0; i < m_types.size(); i++) {
        if (m_names[i] == member) {
            return offset;
        }

        offset += m_types[i]->getSize();
    }

    return (unsigned)-1;
}


unsigned CompoundType::getOffsetRemainder(unsigned n)
{
    unsigned r      = n;
    unsigned offset = 0;

    for (auto& elem : m_types) {
        offset += elem->getSize();

        if (offset > n) {
            break;
        }

        r -= elem->getSize();
    }

    return r;
}


bool CompoundType::operator==(const Type& other) const
{
    const CompoundType& cother = (CompoundType&)other;

    if (other.isCompound() && (cother.m_types.size() == m_types.size())) {
        for (unsigned i = 0; i < m_types.size(); i++) {
            if (!(*m_types[i] == *cother.m_types[i])) {
                return false;
            }
        }

        return true;
    }

    return false;
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


SharedExp CompoundType::match(SharedType pattern)
{
    return Type::match(pattern);
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


void CompoundType::updateGenericMember(int off, SharedType ty, bool& ch)
{
    assert(m_isGeneric);
    int        bit_offset   = off * 8;
    SharedType existingType = getTypeAtOffset(bit_offset);

    if (existingType) {
        existingType = existingType->meetWith(ty, ch);
        setTypeAtOffset(bit_offset, existingType);
    }
    else {
        QString nam = QString("member") + QString::number(m_nextGenericMemberNum++);
        setTypeAtOffset(bit_offset, ty);
        setNameAtOffset(bit_offset, nam);
    }
}


bool CompoundType::isGeneric() const
{
    return m_isGeneric;
}


void CompoundType::addType(SharedType memberType, const QString& memberName)
{
    // check if it is a user defined type (typedef)
    SharedType existingType = getNamedType(memberType->getCtype());

    if (existingType != nullptr) {
        memberType = existingType;
    }

    m_types.push_back(memberType);
    m_names.push_back(memberName);
}


SharedType CompoundType::getType(unsigned int idx)
{
    assert(idx < getNumTypes());
    return m_types[idx];
}


QString CompoundType::getName(unsigned int idx)
{
    assert(idx < getNumTypes());
    return m_names[idx];
}
