#include "SectionInfo.h"

#include <boost/icl/interval_set.hpp>

struct SectionInfoImpl {
    boost::icl::interval_set<ADDRESS> HasDefinedValue;
    void clearDefinedArea() {
        HasDefinedValue.clear();
    }
    void addDefinedArea(ADDRESS from, ADDRESS to) {
        HasDefinedValue.insert(boost::icl::interval<ADDRESS>::right_open(from,to));
    }
    bool isAddressBss(ADDRESS a) const {
        assert(!HasDefinedValue.empty());
        return HasDefinedValue.find(a)==HasDefinedValue.end();
    }
};


// This struct used to be initialised with a memset, but now that overwrites the virtual table (if compiled under gcc
// and possibly others)
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

void SectionInfo::clearDefinedArea() {
    Impl->clearDefinedArea();
}

void SectionInfo::addDefinedArea(ADDRESS from, ADDRESS to) {
    Impl->addDefinedArea(from,to);
}
