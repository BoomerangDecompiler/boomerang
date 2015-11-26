#ifndef SECTIONINFO_H
#define SECTIONINFO_H
#include "types.h"
#include "IBinarySection.h"

#include <QString>
class QVariant;
struct SectionInfoImpl;
//! SectionInfo structure - All information about the sections is contained in these
//! structures.
struct SectionInfo : public IBinarySection {
private:
    SectionInfo &operator=(const SectionInfo &other);
public:
    QString     pSectionName;         // Name of section
    ADDRESS     uNativeAddr;        // Logical or native load address
    ADDRESS     uHostAddr;          // Host or actual address of data
    uint32_t    uSectionSize;      // Size of section in bytes
    uint32_t    uSectionEntrySize; // Size of one section entry (if applicable)
    unsigned    uType;             // Type of section (format dependent)
    unsigned    bCode : 1;         // Set if section contains instructions
    unsigned    bData : 1;         // Set if section contains data
    unsigned    bBss : 1;          // Set if section is BSS (allocated only)
    unsigned    bReadOnly : 1;     // Set if this is a read only section
    uint8_t     Endiannes;          // 0 Little endian, 1 Big endian

    SectionInfo(const QString &name="");    // Constructor
    SectionInfo(const SectionInfo &other);
    virtual ~SectionInfo();
    ADDRESS  hostAddr()     const override { return uHostAddr; }
    ADDRESS  sourceAddr()   const override { return uNativeAddr; }
    uint8_t  getEndian()    const override { return Endiannes; }
    bool     isReadOnly()   const override { return bReadOnly; }
    bool     isCode()       const override { return bCode; }
    bool     isData()       const override { return bData; }
    uint32_t size()         const override { return uSectionSize; }
    QString  getName()      const override { return pSectionName; }
    uint32_t getEntrySize() const override { return uSectionEntrySize; }

    IBinarySection &setBss(bool v) override { bBss = v; return *this; }
    IBinarySection &setCode(bool v) override { bCode = v;  return *this; }
    IBinarySection &setData(bool v) override { bData = v; return *this; }
    IBinarySection &setReadOnly(bool v) override { bReadOnly = v; return *this; }
    IBinarySection &setHostAddr(ADDRESS v) override { uHostAddr = v; return *this; }
    IBinarySection &setEntrySize(uint32_t v) override { uSectionEntrySize = v; return *this; }
    IBinarySection &setEndian(uint8_t v) override { Endiannes = v; return *this; }

    bool        isAddressBss(ADDRESS a) const override;
    bool        anyDefinedValues() const override;
    void        resize(uint32_t ) override;

    void        clearDefinedArea();
    void        addDefinedArea(ADDRESS from,ADDRESS to) override;
    void        setAttributeForRange(const QString &name,const QVariant &val,ADDRESS from,ADDRESS to) override;
    QVariantMap getAttributesForRange(ADDRESS from,ADDRESS to) override;
    QVariant    attributeInRange(const QString &attrib,ADDRESS from,ADDRESS to) const;
private:
    SectionInfoImpl *Impl;
};


#endif // SECTIONINFO_H
