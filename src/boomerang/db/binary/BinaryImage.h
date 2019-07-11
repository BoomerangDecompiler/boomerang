#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/util/Address.h"
#include "boomerang/util/IntervalMap.h"

#include <QByteArray>

#include <memory>
#include <vector>


class BinarySection;


/**
 * This class provides file-format independent access to sections and code/data
 * for binary files.
 *
 * \sa BinaryFile
 * \sa BinarySection
 */
class BOOMERANG_API BinaryImage
{
    typedef std::vector<BinarySection *> SectionList;
    typedef SectionList::iterator iterator;
    typedef SectionList::const_iterator const_iterator;
    typedef SectionList::reverse_iterator reverse_iterator;
    typedef SectionList::const_reverse_iterator const_reverse_iterator;

public:
    BinaryImage(const QByteArray &rawData);
    BinaryImage(const BinaryImage &other) = delete;
    BinaryImage(BinaryImage &&other)      = delete;

    virtual ~BinaryImage();

    BinaryImage &operator=(const BinaryImage &other) = delete;
    BinaryImage &operator=(BinaryImage &&other) = delete;

public:
    // Section iteration
    iterator begin() { return m_sections.begin(); }
    iterator end() { return m_sections.end(); }
    const_iterator begin() const { return m_sections.begin(); }
    const_iterator end() const { return m_sections.end(); }

    reverse_iterator rbegin() { return m_sections.rbegin(); }
    reverse_iterator rend() { return m_sections.rend(); }
    const_reverse_iterator rbegin() const { return m_sections.rbegin(); }
    const_reverse_iterator rend() const { return m_sections.rend(); }

public:
    QByteArray &getRawData() { return m_rawData; }
    const QByteArray &getRawData() const { return m_rawData; }

    /// \returns the number of sections in this image
    int getNumSections() const { return m_sections.size(); }

    bool hasSections() const { return !m_sections.empty(); }

    /// Removes all sections from this image.
    void reset();

    /// Creates a new section with name \p name between \p from and \p to
    /// \returns the new section, or nullptr on failure.
    BinarySection *createSection(const QString &name, Address from, Address to);
    BinarySection *createSection(const QString &name, Interval<Address> extent);

    /// \returns the section with index \p idx, or nullptr if not found.
    BinarySection *getSectionByIndex(int idx);
    const BinarySection *getSectionByIndex(int idx) const;

    /// \returns the section with name \p sectionName, or nullptr if not found.
    BinarySection *getSectionByName(const QString &sectionName);
    const BinarySection *getSectionByName(const QString &sectionName) const;

    /// \returns the section containing address \p addr, or nullptr if not found.
    BinarySection *getSectionByAddr(Address addr);
    const BinarySection *getSectionByAddr(Address addr) const;

    /// After creating (a) section(s), update the section limits
    /// beyond which no code or data exists
    void updateTextLimits();

    /// \returns the low limit of all sections.
    /// If no such sections exist, return Address::INVALID
    Address getLimitTextLow() const;

    /// \returns the high limit of all sections.
    /// If no such sections exist, return Address::INVALID
    Address getLimitTextHigh() const;

    Interval<Address> getLimitText() const;

    ptrdiff_t getTextDelta() const { return m_textDelta; }

    bool readNative1(Address addr, Byte &value) const;
    bool readNative2(Address addr, SWord &value) const;
    bool readNative4(Address addr, DWord &value) const;
    bool readNative8(Address addr, QWord &value) const;

    bool readNativeAddr4(Address addr, Address &value) const;
    bool readNativeAddr8(Address addr, Address &value) const;

    bool readNativeFloat4(Address addr, float &value) const;
    bool readNativeFloat8(Address addr, double &value) const;

    bool writeNative4(Address addr, DWord value);

    /// \returns true if \p addr is in a read-only section
    bool isReadOnly(Address addr) const;

private:
    QByteArray m_rawData;
    Address m_limitTextLow  = Address::INVALID;
    Address m_limitTextHigh = Address::INVALID;
    ptrdiff_t m_textDelta   = 0;

    SectionList m_sections; ///< The section info
    IntervalMap<Address, std::unique_ptr<BinarySection>> m_sectionMap;
};
