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


#include "boomerang/db/binary/SectionInfo.h"
#include "boomerang/db/binary/BinaryImage.h"

#include "boomerang/util/IntervalMap.h"


class BinaryImage
{
    typedef std::vector<IBinarySection *>     SectionListType;
    typedef SectionListType::iterator         iterator;
    typedef SectionListType::const_iterator   const_iterator;

public:
    BinaryImage();
    BinaryImage(const BinaryImage& other) = delete;
    BinaryImage(BinaryImage&& other) = default;

    virtual ~BinaryImage();

    BinaryImage& operator=(const BinaryImage& other) = delete;
    BinaryImage& operator=(BinaryImage&& other) = default;

public:
    /// \returns the number of sections in this image
    size_t size() const { return m_sections.size(); }

    /// \returns whether this image contains any sections
    bool empty()  const { return m_sections.empty(); }

    /// Removes all sections from this image.
    void reset();

    /// Creates a new section with name \p name between \p from and \p to
    IBinarySection *createSection(const QString& name, Address from, Address to);

    /// \copydoc BinaryImage::getSectionInfo
    const IBinarySection *getSection(int idx) const { return m_sections[idx]; }

    /// \copydoc BinaryImage::getSectionInfoByName
    IBinarySection *getSectionByName(const QString& sectionName);

    /// \copydoc BinaryImage::getSectionByAddr
    const IBinarySection *getSectionByAddr(Address addr) const;

    /// \copydoc BinaryImage::getSectionIndexByName
    int getSectionIndex(const QString& sectionName);

    /// \copydoc BinaryImage::getNumSections
    size_t getNumSections() const { return m_sections.size(); }

    // Section iteration
    iterator begin()             { return m_sections.begin(); }
    const_iterator begin() const { return m_sections.begin(); }
    iterator end()               { return m_sections.end(); }
    const_iterator end()   const { return m_sections.end(); }

    /// \copydoc BinaryImage::updateTextLimits
    void updateTextLimits();

    /// \copydoc BinaryImage::getLimitTextLow
    Address getLimitTextLow() const;

    /// \copydoc BinaryImage::getLimitTextHigh
    Address getLimitTextHigh() const;

    /// \copydoc BinaryImage::getTextDelta
    ptrdiff_t getTextDelta() const { return m_textDelta; }


    Byte readNative1(Address addr);               ///< \copydoc BinaryImage::readNative1
    SWord readNative2(Address addr);              ///< \copydoc BinaryImage::readNative2
    DWord readNative4(Address addr);              ///< \copydoc BinaryImage::readNative4
    QWord readNative8(Address addr);              ///< \copydoc BinaryImage::readNative8
    float readNativeFloat4(Address addr);         ///< \copydoc BinaryImage::readNativeFloat4
    double readNativeFloat8(Address addr);        ///< \copydoc BinaryImage::readNativeFloat8
    void writeNative4(Address addr, DWord value); ///< \copydoc BinaryImage::writeNative4

    /// \copydoc BinaryImage::isReadOnly
    bool isReadOnly(Address addr);

private:
    Address m_limitTextLow;
    Address m_limitTextHigh;
    ptrdiff_t m_textDelta;
    IntervalMap<Address, std::unique_ptr<IBinarySection>> m_sectionMap;
    SectionListType m_sections; ///< The section info
};
