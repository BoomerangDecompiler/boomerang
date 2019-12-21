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

#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"

#include <algorithm>


BinaryImage::BinaryImage(const QByteArray &rawData)
    : m_rawData(rawData)
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


bool BinaryImage::readNative1(Address addr, Byte &value) const
{
    const BinarySection *section = getSectionByAddr(addr);

    if (section == nullptr || section->getHostAddr() == HostAddress::INVALID) {
        LOG_WARN("Invalid read at address %1: Address is not mapped to a section", addr);
        return false;
    }

    HostAddress host = section->getHostAddr() - section->getSourceAddr() + addr;
    value            = *reinterpret_cast<Byte *>(host.value());
    return true;
}


bool BinaryImage::readNative2(Address addr, SWord &value) const
{
    const BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr || si->getHostAddr() == HostAddress::INVALID) {
        LOG_WARN("Invalid read at address %1: Address is not mapped to a section", addr.toString());
        return false;
    }
    else if (addr + 2 > si->getSourceAddr() + si->getSize()) {
        LOG_WARN("Invalid read at address %1: Read extends past section boundary", addr);
        return false;
    }
    else if (si->isAddressBss(addr)) {
        return false;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;
    value = Util::readWord(reinterpret_cast<const Byte *>(host.value()), si->getEndian());
    return true;
}


bool BinaryImage::readNative4(Address addr, DWord &value) const
{
    const BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr || si->getHostAddr() == HostAddress::INVALID) {
        LOG_WARN("Invalid read at address %1: Address is not mapped to a section", addr.toString());
        return false;
    }
    else if (addr + 4 > si->getSourceAddr() + si->getSize()) {
        LOG_WARN("Invalid read at address %1: Read extends past section boundary", addr);
        return false;
    }
    else if (si->isAddressBss(addr)) {
        return false;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;
    value = Util::readDWord(reinterpret_cast<const Byte *>(host.value()), si->getEndian());
    return true;
}


bool BinaryImage::readNative8(Address addr, QWord &value) const
{
    const BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr || si->getHostAddr() == HostAddress::INVALID) {
        LOG_WARN("Invalid read at address %1: Address is not mapped to a section", addr.toString());
        return false;
    }
    else if (addr + 8 > si->getSourceAddr() + si->getSize()) {
        LOG_WARN("Invalid read at address %1: Read extends past section boundary", addr);
        return false;
    }
    else if (si->isAddressBss(addr)) {
        return false;
    }

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;
    value = Util::readQWord(reinterpret_cast<const Byte *>(host.value()), si->getEndian());
    return true;
}


bool BinaryImage::readNativeAddr4(Address addr, Address &value) const
{
    assert(Address::getSourceBits() == 32);
    DWord val = value.value() & Address::getSourceMask();
    if (readNative4(addr, val)) {
        value = Address(val);
        return true;
    }

    return false;
}


bool BinaryImage::readNativeAddr8(Address addr, Address &value) const
{
    assert(Address::getSourceBits() == 64);
    QWord val = value.value() & Address::getSourceMask();
    if (readNative8(addr, val)) {
        value = Address(val);
        return true;
    }

    return false;
}


bool BinaryImage::readNativeFloat4(Address addr, float &value) const
{
    const BinarySection *sect = getSectionByAddr(addr);
    DWord raw                 = 0;

    if (sect == nullptr || sect->getHostAddr() == HostAddress::INVALID) {
        LOG_WARN("Invalid read at address %1: Address is not mapped to a section", addr.toString());
        return false;
    }
    else if (addr + 4 > sect->getSourceAddr() + sect->getSize()) {
        LOG_WARN("Invalid read at address %1: Read extends past section boundary", addr);
        return false;
    }
    else if (!readNative4(addr, raw)) {
        return false;
    }

    value = *reinterpret_cast<float *>(&raw); // Note: cast, not convert
    return true;
}


bool BinaryImage::readNativeFloat8(Address addr, double &value) const
{
    const BinarySection *sect = getSectionByAddr(addr);
    QWord raw                 = 0;

    if (sect == nullptr || sect->getHostAddr() == HostAddress::INVALID) {
        LOG_WARN("Invalid read at address %1: Address is not mapped to a section", addr.toString());
        return false;
    }
    else if (addr + 8 > sect->getSourceAddr() + sect->getSize()) {
        LOG_WARN("Invalid read at address %1: Read extends past section boundary", addr);
        return false;
    }
    else if (!readNative8(addr, raw)) {
        return false;
    }

    value = *reinterpret_cast<double *>(&raw);
    return true;
}


bool BinaryImage::writeNative4(Address addr, uint32_t value)
{
    BinarySection *si = getSectionByAddr(addr);

    if (si == nullptr || si->getHostAddr() == HostAddress::INVALID) {
        LOG_WARN("Ignoring write at address %1: Address is outside any writable section");
        return false;
    }
    else if (addr + 4 > si->getSourceAddr() + si->getSize()) {
        LOG_WARN("Invalid write at address %1: Write extends past section boundary", addr);
        return false;
    }

    si->addDefinedArea(addr, addr + 4);

    HostAddress host = si->getHostAddr() - si->getSourceAddr() + addr;
    Util::writeDWord(reinterpret_cast<void *>(host.value()), value, si->getEndian());
    return true;
}


void BinaryImage::updateTextLimits()
{
    m_limitTextLow  = Address::INVALID;
    m_limitTextHigh = Address::INVALID;
    m_textDelta     = 0;

    for (BinarySection *section : m_sections) {
        if (!section->isCode()) {
            continue;
        }

        // The .plt section is an anomaly. It's code, but we never want to
        // decode it. For now, we make this ugly exception
        if (section->getName() == ".plt") {
            continue;
        }

        if (m_limitTextLow == Address::INVALID || section->getSourceAddr() < m_limitTextLow) {
            m_limitTextLow = section->getSourceAddr();
        }

        const Address highAddress = section->getSourceAddr() + section->getSize();

        if (m_limitTextHigh == Address::INVALID || highAddress > m_limitTextHigh) {
            m_limitTextHigh = highAddress;
        }

        const ptrdiff_t hostNativeDiff = (section->getHostAddr() - section->getSourceAddr())
                                             .value();

        if (m_textDelta == 0) {
            m_textDelta = hostNativeDiff;
        }
        else if (m_textDelta != hostNativeDiff) {
            LOG_WARN("TextDelta different for section %1 (ignoring).", section->getName());
        }
    }
}


bool BinaryImage::isReadOnly(Address addr) const
{
    const BinarySection *section = getSectionByAddr(addr);

    if (!section) {
        return false;
    }

    if (section->isReadOnly()) {
        return true;
    }

    return section->isAttributeInRange("ReadOnly", addr, addr + 1);
}


Address BinaryImage::getLimitTextLow() const
{
    return m_limitTextLow;
}


Address BinaryImage::getLimitTextHigh() const
{
    return m_limitTextHigh;
}


Interval<Address> BinaryImage::getLimitText() const
{
    return Interval<Address>(m_limitTextLow, m_limitTextHigh);
}


BinarySection *BinaryImage::createSection(const QString &name, Address from, Address to)
{
    if (from == Address::INVALID || to == Address::INVALID || to < from) {
        LOG_ERROR("Could not create section '%1' with invalid extent [%2, %3)", name, from, to);
        return nullptr;
    }
    else if (getSectionByName(name) != nullptr) {
        LOG_ERROR("Could not create section '%1': A section with the same name already exists",
                  name);
        return nullptr;
    }

    if (from == to) {
        to += 1; // open interval, so -> [from,to+1) is right
    }

#if DEBUG
    // see
    // https://stackoverflow.com/questions/25501044/gcc-ld-overlapping-sections-tbss-init-array-in-statically-linked-elf-bin
    // Basically, the .tbss section is of type SHT_NOBITS, so there is no data associated to the
    // section. It can therefore overlap other sections containing data. This is a quirk of ELF
    // programs linked statically with glibc
    if (name != ".tbss") {
        IntervalMap<Address, std::unique_ptr<BinarySection>>::iterator itFrom, itTo;
        std::tie(itFrom, itTo) = m_sectionMap.equalRange(from, to);

        for (auto clash_with = itFrom; clash_with != itTo; ++clash_with) {
            if ((*clash_with->second).getName() != ".tbss") {
                LOG_WARN("Segment %1 would intersect existing segment %2", name,
                         (*clash_with->second).getName());
                return nullptr;
            }
        }
    }
#endif

    BinarySection *sect = new BinarySection(from, (to - from).value(), name);

    if (m_sectionMap.insert(from, to, std::unique_ptr<BinarySection>(sect)) == m_sectionMap.end()) {
        // section already existed
        BinarySection *existing = getSectionByAddr(from);
        LOG_ERROR("Could not create section '%1' from address %2 to %3: Section extent matches "
                  "existing section '%4'",
                  name, from, to, existing ? existing->getName() : "<invalid>");
        return nullptr;
    }
    else {
        m_sections.push_back(sect);
        return sect;
    }
}


BinarySection *BinaryImage::createSection(const QString &name, Interval<Address> extent)
{
    return createSection(name, extent.lower(), extent.upper());
}


BinarySection *BinaryImage::getSectionByIndex(int idx)
{
    return Util::inRange(idx, 0, getNumSections()) ? m_sections[idx] : nullptr;
}


const BinarySection *BinaryImage::getSectionByIndex(int idx) const
{
    return Util::inRange(idx, 0, getNumSections()) ? m_sections[idx] : nullptr;
}


BinarySection *BinaryImage::getSectionByName(const QString &sectionName)
{
    for (BinarySection *section : m_sections) {
        if (section->getName() == sectionName) {
            return section;
        }
    }

    return nullptr;
}


const BinarySection *BinaryImage::getSectionByName(const QString &sectionName) const
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
