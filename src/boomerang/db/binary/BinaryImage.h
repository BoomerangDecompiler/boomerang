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


#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/util/IntervalMap.h"

#include <memory>


class BinaryImage
{
    typedef std::vector<BinarySection *> SectionList;
    typedef SectionList::iterator               iterator;
    typedef SectionList::const_iterator         const_iterator;
    typedef SectionList::reverse_iterator       reverse_iterator;
    typedef SectionList::const_reverse_iterator const_reverse_iterator;

public:
    BinaryImage();
    BinaryImage(const BinaryImage& other) = delete;
    BinaryImage(BinaryImage&& other) = default;

    virtual ~BinaryImage();

    BinaryImage& operator=(const BinaryImage& other) = delete;
    BinaryImage& operator=(BinaryImage&& other) = default;

public:
    // Section iteration
    iterator begin()             { return m_sections.begin(); }
    iterator end()               { return m_sections.end(); }
    const_iterator begin() const { return m_sections.begin(); }
    const_iterator end()   const { return m_sections.end(); }

    reverse_iterator rbegin()             { return m_sections.rbegin(); }
    reverse_iterator rend()               { return m_sections.rend(); }
    const_reverse_iterator rbegin() const { return m_sections.rbegin(); }
    const_reverse_iterator rend()   const { return m_sections.rend(); }

public:
    /// \returns the number of sections in this image
    int getNumSections() const { return m_sections.size(); }

    bool hasSections() const { return !m_sections.empty(); }

    /// Removes all sections from this image.
    void reset();

    /// Creates a new section with name \p name between \p from and \p to
    BinarySection *createSection(const QString& name, Address from, Address to);
    BinarySection *createSection(const QString& name, Interval<Address> extent);

    BinarySection *getSectionByIndex(int idx);
    const BinarySection *getSectionByIndex(int idx) const;

    BinarySection *getSectionByName(const QString& sectionName);
    const BinarySection *getSectionByName(const QString& sectionName) const;

    BinarySection *getSectionByAddr(Address addr);
    const BinarySection *getSectionByAddr(Address addr) const;

    void updateTextLimits();

    Address getLimitTextLow() const;

    Address getLimitTextHigh() const;

    ptrdiff_t getTextDelta() const { return m_textDelta; }


    Byte readNative1(Address addr);
    SWord readNative2(Address addr);
    DWord readNative4(Address addr);
    QWord readNative8(Address addr);
    float readNativeFloat4(Address addr);
    double readNativeFloat8(Address addr);
    void writeNative4(Address addr, DWord value);

    bool isReadOnly(Address addr);

private:
    Address m_limitTextLow;
    Address m_limitTextHigh;
    ptrdiff_t m_textDelta;
    IntervalMap<Address, std::unique_ptr<BinarySection>> m_sectionMap;
    SectionList m_sections; ///< The section info
};
