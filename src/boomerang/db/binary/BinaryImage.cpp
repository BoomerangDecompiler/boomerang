#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinaryImage.h"


#include "boomerang/util/Types.h"
#include "boomerang/util/Log.h"

#include <algorithm>


BinaryImage::BinaryImage()
{
}


BinaryImage::~BinaryImage()
{
    reset();
}


void BinaryImage::reset()
{
    m_sectionMap.clear();
    m_sections.clear();
}


Byte BinaryImage::readNative1(Address addr)
{
    const BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr) {
        LOG_WARN("Target Memory access in unmapped section at address %1", addr.toString());
        return 0xFF;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;
    return *reinterpret_cast<Byte *>(host.value());
}


SWord BinaryImage::readNative2(Address nat)
{
    const BinarySection *si = getSectionByAddr(nat);

    if (si == nullptr) {
        return 0;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + nat;
    return Util::readWord(reinterpret_cast<const Byte *>(host.value()), si->getEndian());
}


DWord BinaryImage::readNative4(Address addr)
{
    const BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr) {
        return 0;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;
    return Util::readDWord(reinterpret_cast<const Byte *>(host.value()), si->getEndian());
}


QWord BinaryImage::readNative8(Address addr)
{
    const BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr) {
        return 0;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;
    return Util::readQWord(reinterpret_cast<const Byte *>(host.value()), si->getEndian());
}


float BinaryImage::readNativeFloat4(Address nat)
{
    DWord raw = readNative4(nat);

    return *reinterpret_cast<float *>(&raw); // Note: cast, not convert
}


double BinaryImage::readNativeFloat8(Address nat)
{
    const BinarySection *si = getSectionByAddr(nat);

    if (si == nullptr) {
        return 0;
    }

    QWord raw = readNative8(nat);
    return *reinterpret_cast<double *>(&raw);
}


void BinaryImage::writeNative4(Address addr, uint32_t value)
{
    const BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr) {
        LOG_WARN("Ignoring write at address %1: Address is outside any known section");
        return;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;

    Util::writeDWord(reinterpret_cast<void *>(host.value()), value, si->getEndian());
}


void BinaryImage::updateTextLimits()
{
    m_limitTextLow  = Address::INVALID;
    m_limitTextHigh = Address::ZERO;
    m_textDelta     = 0;

    for (BinarySection *section : m_sections) {
        if (!section->isCode()) {
            continue;
        }

        // The .plt section is an anomaly. It's code, but we never want to
        // decode it, and in Sparc ELF files, it's actually in the data
        // section (so it can be modified). For now, we make this ugly
        // exception
        if (".plt" == section->getName()) {
            continue;
        }

        if (section->getSourceAddr() < m_limitTextLow) {
            m_limitTextLow = section->getSourceAddr();
        }

        const Address highAddress = section->getSourceAddr() + section->getSize();

        if (highAddress > m_limitTextHigh) {
            m_limitTextHigh = highAddress;
        }

        const ptrdiff_t hostNativeDiff = (section->getHostAddr() - section->getSourceAddr()).value();

        if (m_textDelta == 0) {
            m_textDelta = hostNativeDiff;
        }
        else if (m_textDelta != hostNativeDiff) {
            LOG_WARN("TextDelta different for section %s (ignoring).\n", qPrintable(section->getName()));
        }
    }
}


bool BinaryImage::isReadOnly(Address addr)
{
    const BinarySection *p = static_cast<const BinarySection *>(getSectionByAddr(addr));

    if (!p) {
        return false;
    }

    if (p->isReadOnly()) {
        return true;
    }

    QVariant v = p->attributeInRange("ReadOnly", addr, addr + 1);
    return !v.isNull();
}


Address BinaryImage::getLimitTextLow() const
{
    return m_sectionMap.begin()->first.lower();
}


Address BinaryImage::getLimitTextHigh() const
{
    return m_sectionMap.rbegin()->first.upper();
}


BinarySection *BinaryImage::createSection(const QString& name, Address from, Address to)
{
    assert(from <= to);

    if (from == to) {
        to += 1; // open interval, so -> [from,to+1) is right
    }

#if DEBUG
    // see https://stackoverflow.com/questions/25501044/gcc-ld-overlapping-sections-tbss-init-array-in-statically-linked-elf-bin
    // Basically, the .tbss section is of type SHT_NOBITS, so there is no data associated to the section.
    // It can therefore overlap other sections containing data.
    // This is a quirk of ELF programs linked statically with glibc
    if (name != ".tbss") {
        SectionRangeMap::iterator itFrom, itTo;
        std::tie(itFrom, itTo) = m_sectionMap.equalRange(from, to);

        for (SectionRangeMap::iterator clash_with = itFrom; clash_with != itTo; ++clash_with) {
            if ((*clash_with->second).getName() != ".tbss") {
                LOG_WARN("Segment %1 would intersect existing segment %2", name, (*clash_with->second).getName());
                return nullptr;
            }
        }
    }
#endif

    BinarySection *sect = new BinarySection(from, (to - from).value(), name);
    m_sections.push_back(sect);

    m_sectionMap.insert(from, to, std::unique_ptr<BinarySection>(sect));
    return sect;
}


BinarySection *BinaryImage::createSection(const QString& name, Interval<Address> extent)
{
    return createSection(name, extent.lower(), extent.upper());
}


BinarySection *BinaryImage::getSectionByIndex(int idx)
{
    assert(Util::inRange(idx, 0, getNumSections()));
    return m_sections[idx];
}

const BinarySection *BinaryImage::getSectionByIndex(int idx) const
{
    assert(Util::inRange(idx, 0, getNumSections()));
    return m_sections[idx];
}


BinarySection *BinaryImage::getSectionByName(const QString& sectionName)
{
    for (BinarySection *section : m_sections) {
        if (section->getName() == sectionName) {
            return section;
        }
    }

    return nullptr;
}


const BinarySection *BinaryImage::getSectionByName(const QString& sectionName) const
{
    for (const BinarySection *section : m_sections) {
        if (section->getName() == sectionName) {
            return section;
        }
    }

    return nullptr;
}


BinarySection *BinaryImage::getSectionByAddr(Address addr)
{
    auto iter = m_sectionMap.find(addr);
    return (iter != m_sectionMap.end()) ? iter->second.get() : nullptr;
}


const BinarySection *BinaryImage::getSectionByAddr(Address addr) const
{
    auto iter = m_sectionMap.find(addr);
    return (iter != m_sectionMap.end()) ? iter->second.get() : nullptr;
}


