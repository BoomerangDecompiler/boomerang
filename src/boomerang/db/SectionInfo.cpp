#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SectionInfo.h"


#include "boomerang/util/Log.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/util/IntervalMap.h"
#include "boomerang/util/IntervalSet.h"

#include <QVariantMap>

#include <algorithm>
#include <utility>


struct VariantHolder
{
    mutable QVariantMap val;
    QVariantMap&        get() const { return val; }
    VariantHolder& operator+=(const VariantHolder& other)
    {
        val = val.unite(other.val);
        return *this;
    }

    bool operator==(const VariantHolder& other) const
    {
        return val == other.val;
    }
};

class SectionInfoImpl
{
public:
    void clearDefinedArea()
    {
        m_hasDefinedValue.clear();
    }

    void addDefinedArea(Address from, Address to)
    {
        m_hasDefinedValue.insert(from, to);
    }

    bool isAddressBss(Address a) const
    {
        assert(!m_hasDefinedValue.isEmpty());
        return !m_hasDefinedValue.isContained(a);
    }

    void setAttributeForRange(const QString& name, const QVariant& val, Address from, Address to)
    {
        QVariantMap vmap;

        vmap[name] = val;
        VariantHolder map { vmap };
        m_attributeMap.insert(from, to, map);
    }

    QVariant attributeInRange(const QString& attrib, Address from, Address to) const
    {
        auto startIt = m_attributeMap.find(from);
        auto endIt   = m_attributeMap.find(to);

        if (startIt == m_attributeMap.end()) {
            return QVariant();
        }

        QList<QVariant> vals;

        for (auto iter = startIt; iter != endIt; ++iter) {
            if (iter->second.get().contains(attrib)) {
                vals << iter->second.get()[attrib];
            }
        }

        if (vals.size() == 1) {
            return vals.front();
        }

        return QVariant(vals);
    }

    QVariantMap getAttributesForRange(Address from, Address to)
    {
        QVariantMap res;
        auto        v = m_attributeMap.equalRange(from, to);

        if (v.first == m_attributeMap.end()) {
            return res;
        }

        for (auto iter = v.first; iter != v.second; ++iter) {
            res.unite(iter->second.get());
        }

        return res;
    }

public:
    IntervalSet<Address> m_hasDefinedValue;
    IntervalMap<Address, VariantHolder> m_attributeMap;
};



SectionInfo::SectionInfo(Address sourceAddr, uint64 size, const QString& name)
    : m_impl(new SectionInfoImpl)
    , m_sectionName(name)
    , m_nativeAddr(sourceAddr)
    , m_hostAddr(HostAddress::ZERO)
    , m_size(size)
    , m_sectionEntrySize(0)
    , m_code(false)
    , m_data(false)
    , m_bss(0)
    , m_readOnly(0)
{
}


SectionInfo::SectionInfo(const SectionInfo& other)
    : SectionInfo(other.m_nativeAddr, other.m_size, other.m_sectionName)
{
    *m_impl            = *other.m_impl;
    m_hostAddr         = other.m_hostAddr;
    m_sectionEntrySize = other.m_sectionEntrySize;
    m_code             = other.m_code;
    m_data             = other.m_data;
    m_bss              = other.m_bss;
    m_readOnly         = other.m_readOnly;
}


SectionInfo::~SectionInfo()
{
    delete m_impl;
}


bool SectionInfo::isAddressBss(Address a) const
{
    assert(a >= m_nativeAddr && a < m_nativeAddr + m_size);

    if (m_bss) {
        return true;
    }

    if (m_readOnly) {
        return false;
    }

    return m_impl->isAddressBss(a);
}


bool SectionInfo::anyDefinedValues() const
{
    return !m_impl->m_hasDefinedValue.isEmpty();
}


void SectionInfo::resize(uint32_t sz)
{
    LOG_VERBOSE("Function not fully implemented yet");
    m_size = sz;

//    assert(false && "This function is not implmented yet");
//    if(sz!=uSectionSize) {
//        const IBinarySection *sect = Boomerang::get()->getImage()->getSectionByAddr(uNativeAddr+sz);
//        if(sect==nullptr || sect==this ) {
//        }
//    }
}


void SectionInfo::clearDefinedArea()
{
    m_impl->clearDefinedArea();
}


void SectionInfo::addDefinedArea(Address from, Address to)
{
    m_impl->addDefinedArea(from, to);
}


void SectionInfo::setAttributeForRange(const QString& name, const QVariant& val, Address from, Address to)
{
    m_impl->setAttributeForRange(name, val, from, to);
}


QVariantMap SectionInfo::getAttributesForRange(Address from, Address to)
{
    return m_impl->getAttributesForRange(from, to);
}


QVariant SectionInfo::attributeInRange(const QString& attrib, Address from, Address to) const
{
    return m_impl->attributeInRange(attrib, from, to);
}
