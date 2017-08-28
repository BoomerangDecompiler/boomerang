#pragma once

#include "boomerang/util/Address.h"

class IBinarySection;
class QString;

class IBinaryImage
{
public:
    typedef std::vector<IBinarySection*>      SectionListType;
    typedef SectionListType::iterator         iterator;
    typedef SectionListType::const_iterator   const_iterator;

public:
    IBinaryImage() {}
    virtual ~IBinaryImage() {}

    /// \returns the number of bytes in this image.
    virtual size_t size() const = 0;

    /// \returns true if this image is empty.
    virtual bool empty()  const = 0;

    /// Unloads all data in this image.
    virtual void reset()        = 0;

    /// Creates a new section with name \p name between \p from and \p to
    virtual IBinarySection* createSection(const QString& name, Address from, Address to) = 0;

    /// Get a section by its index
    virtual const IBinarySection* getSection(int idx) const = 0;

    /// Get the section with name \p sectionName
    virtual IBinarySection* getSectionByName(const QString& sectionName) = 0;

    /// Get the section the address \p addr is in.
    /// If the address does not belong to a section, this function returns nullptr.
    virtual const IBinarySection* getSectionByAddr(Address addr) const = 0;

    /// Get the index of the section with name \p sectionName
    /// Returns -1 if no section was found.
    virtual int getSectionIndex(const QString& sectionName) = 0;

    /// \returns the number of sections in this image.
    virtual size_t getNumSections() const = 0;

    // Section iteration
    virtual iterator begin()             = 0;
    virtual const_iterator begin() const = 0;
    virtual iterator end()               = 0;
    virtual const_iterator end() const   = 0;


    /// Update the limits of sections containing code
    virtual void updateTextLimits()          = 0;

    /// \returns The low limit of all code sections
    virtual Address getLimitTextLow()  const = 0;

    /// \returns the high limit of all code sections
    virtual Address getLimitTextHigh() const = 0;

    /// Get the host-native difference of the text (code) section.
    virtual ptrdiff_t getTextDelta()   const = 0;

    virtual Byte readNative1(Address addr)              = 0; ///< Read a single byte from the given address
    virtual SWord readNative2(Address addr)             = 0; ///< Read 2 bytes from given native address, considers endianness
    virtual DWord readNative4(Address addr)             = 0; ///< Read 4 bytes from given native address, considers endianness
    virtual QWord readNative8(Address addr)             = 0; ///< Read 8 bytes from given native address, considers endianness
    virtual float readNativeFloat4(Address addr)        = 0; ///< Read 4 bytes as a float; considers endianness
    virtual double readNativeFloat8(Address addr)       = 0; ///< Read 8 bytes as a float; considers endianness
    virtual void writeNative4(Address addr, DWord value) = 0;

    /// \returns true if the given address is in a read only section
    virtual bool isReadOnly(Address addr) = 0;
};
