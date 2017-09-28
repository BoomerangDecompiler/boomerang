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


#include "SectionInfo.h"
#include "boomerang/db/IBinaryImage.h"

#include "boomerang/util/IntervalMap.h"


struct SectionHolder
{
    SectionHolder()
        : val(nullptr) {}
    SectionHolder(SectionInfo *inf)
        : val(inf) {}

    SectionInfo *operator->() { return val; }
    SectionInfo& operator*() const { return *val; }
    operator SectionInfo *() { return val; }
    operator const SectionInfo *() const { return val; }
    SectionInfo *val;
};


class BinaryImage : public IBinaryImage
{
public:
    /// The type for the list of functions.
    typedef IntervalMap<Address, SectionHolder> SectionRangeMap;

public:
    BinaryImage();
    ~BinaryImage();

    /// \copydoc IBinaryImage::size
    size_t size() const override { return m_sections.size(); }

    /// \copydoc IBinaryImage::empty
    bool empty()  const override { return m_sections.empty(); }

    /// \copydoc IBinaryImage::reset
    void reset() override;

    /// \copydoc IBinaryImage::createSection
    IBinarySection* createSection(const QString& name, Address from, Address to) override;

    /// \copydoc IBinaryImage::getSectionInfo
    const IBinarySection* getSection(int idx) const override { return m_sections[idx]; }

    /// \copydoc IBinaryImage::getSectionInfoByName
    IBinarySection* getSectionByName(const QString& sectionName) override;

    /// \copydoc IBinaryImage::getSectionByAddr
    const IBinarySection* getSectionByAddr(Address addr) const override;

    /// \copydoc IBinaryImage::getSectionIndexByName
    int getSectionIndex(const QString& sName) override;

    /// \copydoc IBinaryImage::getNumSections
    size_t getNumSections() const override { return m_sections.size(); }

    // Section iteration
    iterator begin()             override { return m_sections.begin(); }
    const_iterator begin() const override { return m_sections.begin(); }
    iterator end()               override { return m_sections.end(); }
    const_iterator end()   const override { return m_sections.end(); }

    /// \copydoc IBinaryImage::updateTextLimits
    void updateTextLimits() override;

    /// \copydoc IBinaryImage::getLimitTextLow
    Address getLimitTextLow() const override;

    /// \copydoc IBinaryImage::getLimitTextHigh
    Address getLimitTextHigh() const override;

    /// \copydoc IBinaryImage::getTextDelta
    ptrdiff_t getTextDelta() const override { return m_textDelta; }


    Byte  readNative1(Address addr) override;       ///< \copydoc IBinaryImage::readNative1
    SWord readNative2(Address addr) override;       ///< \copydoc IBinaryImage::readNative2
    DWord readNative4(Address addr) override;       ///< \copydoc IBinaryImage::readNative4
    QWord readNative8(Address addr) override;       ///< \copydoc IBinaryImage::readNative8
    float readNativeFloat4(Address addr) override;  ///< \copydoc IBinaryImage::readNativeFloat4
    double readNativeFloat8(Address addr) override; ///< \copydoc IBinaryImage::readNativeFloat8
    void writeNative4(Address addr, DWord value) override; ///< \copydoc IBinaryImage::writeNative4

    /// \copydoc IBinaryImage::isReadOnly
    bool isReadOnly(Address addr) override;

private:
    BinaryImage(const BinaryImage&) = delete;            ///< prevent copy-construction
    BinaryImage& operator=(const BinaryImage&) = delete; ///< prevent assignment

private:
    Address         m_limitTextLow;
    Address         m_limitTextHigh;
    ptrdiff_t       m_textDelta;
    SectionRangeMap m_sectionMap;
    SectionListType m_sections; ///< The section info
};
