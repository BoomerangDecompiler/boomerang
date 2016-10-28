#include "SectionInfo.h"
#include "boomerang.h"
#include "IBinaryImage.h"

#include <QVariantMap>
#include <QDebug>
#include <boost/icl/interval_set.hpp>
#include <boost/icl/interval_map.hpp>
#include <algorithm>
#include <utility>
using namespace boost::icl;
struct VariantHolder {
    mutable QVariantMap val;
    QVariantMap &get() const {return val;}
    VariantHolder &operator+=(const VariantHolder &other) {
        val = val.unite(other.val);
        return *this;
    }
    bool operator==(const VariantHolder &other) const {
        return val==other.val;
    }
};
struct SectionInfoImpl {
    boost::icl::interval_set<ADDRESS> HasDefinedValue;
    boost::icl::interval_map<ADDRESS,VariantHolder> AttributeMap;
    void clearDefinedArea() {
        HasDefinedValue.clear();
    }
    void addDefinedArea(ADDRESS from, ADDRESS to) {
        HasDefinedValue.insert(interval<ADDRESS>::right_open(from,to));
    }
    bool isAddressBss(ADDRESS a) const {
        assert(!HasDefinedValue.empty());
        return HasDefinedValue.find(a)==HasDefinedValue.end();
    }
    void setAttributeForRange(const QString &name, const QVariant &val, ADDRESS from, ADDRESS to) {
        QVariantMap vmap;
        vmap[name] = val;
        VariantHolder map { vmap  };
        AttributeMap.add(std::make_pair(interval<ADDRESS>::right_open(from,to),map));
    }
    QVariant attributeInRange(const QString &attrib, ADDRESS from, ADDRESS to) const {
        auto v = AttributeMap.equal_range(interval<ADDRESS>::right_open(from,to));
        if(v.first==AttributeMap.end())
            return QVariant();
        QList<QVariant> vals;
        for(auto iter=v.first; iter!=v.second; ++iter) {
            if(iter->second.get().contains(attrib)) {
                vals << iter->second.get()[attrib];
            }
        }
        if(vals.size()==1)
            return vals.front();
        return QVariant(vals);
    }
    QVariantMap getAttributesForRange(ADDRESS from, ADDRESS to) {
        QVariantMap res;
        auto v = AttributeMap.equal_range(interval<ADDRESS>::right_open(from,to));
        if(v.first==AttributeMap.end())
            return res;
        for(auto iter=v.first; iter!=v.second; ++iter) {
            res.unite(iter->second.get());
        }
        return res;
    }
};


SectionInfo::SectionInfo(const QString &name)
    : pSectionName(name),uNativeAddr(ADDRESS::g(0L)), uHostAddr(ADDRESS::g(0L)), uSectionSize(0),
      uSectionEntrySize(0), uType(0), bCode(false), bData(false), bBss(0), bReadOnly(0),Impl(new SectionInfoImpl) {}

SectionInfo::SectionInfo(const SectionInfo &other) : SectionInfo(other.pSectionName)
{
    *Impl = *other.Impl;
    uNativeAddr = other.uNativeAddr;
    uHostAddr = other.uHostAddr;
    uSectionSize = other.uSectionSize;
    uSectionEntrySize = other.uSectionEntrySize;
    bCode = other.bCode;
    bData = other.bData;
    bBss = other.bBss;
    bReadOnly = other.bReadOnly;
}
SectionInfo::~SectionInfo() {
    delete Impl;
}
// Windows's PE file sections can contain any combination of code, data and bss.
// As such, it can't be correctly described by SectionInfo, why we need to override
// the behaviour of (at least) the question "Is this address in BSS".
bool SectionInfo::isAddressBss(ADDRESS a) const {
    assert(a>=uNativeAddr && a<uNativeAddr+uSectionSize);
    if(bBss)
        return true;
    if(bReadOnly)
        return false;
    return Impl->isAddressBss(a);
}

bool SectionInfo::anyDefinedValues() const { return !Impl->HasDefinedValue.empty();}

void SectionInfo::resize(uint32_t sz)
{
    qDebug() << "SectionInfo::resize not fully implemented yet";
    uSectionSize = sz;
//    assert(false && "This function is not implmented yet");
//    if(sz!=uSectionSize) {
//        const IBinarySection *sect = Boomerang::get()->getImage()->getSectionInfoByAddr(uNativeAddr+sz);
//        if(sect==nullptr || sect==this ) {
//        }
//    }
}

void SectionInfo::clearDefinedArea() {
    Impl->clearDefinedArea();
}

void SectionInfo::addDefinedArea(ADDRESS from, ADDRESS to) {
    Impl->addDefinedArea(from,to);
}

void SectionInfo::setAttributeForRange(const QString &name, const QVariant &val, ADDRESS from, ADDRESS to)
{
    Impl->setAttributeForRange(name,val,from,to);
}

QVariantMap SectionInfo::getAttributesForRange(ADDRESS from, ADDRESS to)
{
    return Impl->getAttributesForRange(from,to);
}

QVariant SectionInfo::attributeInRange(const QString &attrib, ADDRESS from, ADDRESS to) const
{
    return Impl->attributeInRange(attrib,from,to);
}
