#ifndef SECTIONINFO_H
#define SECTIONINFO_H
#include "types.h"

#include <QString>
struct SectionInfoImpl;
//! SectionInfo structure - All information about the sections is contained in these
//! structures.
struct SectionInfo {
private:
    SectionInfo &operator=(const SectionInfo &other);
public:
    SectionInfo(const QString &name="");    // Constructor
    SectionInfo(const SectionInfo &other);
    ~SectionInfo();
    QString pSectionName;         // Name of section
    ADDRESS uNativeAddr;        // Logical or native load address
    ADDRESS uHostAddr;          // Host or actual address of data
    uint32_t uSectionSize;      // Size of section in bytes
    uint32_t uSectionEntrySize; // Size of one section entry (if applic)
    unsigned uType;             // Type of section (format dependent)
    unsigned bCode : 1;         // Set if section contains instructions
    unsigned bData : 1;         // Set if section contains data
    unsigned bBss : 1;          // Set if section is BSS (allocated only)
    unsigned bReadOnly : 1;     // Set if this is a read only section
    uint8_t Endiannes;          // 0 Little endian, 1 Big endian

    bool isAddressBss(ADDRESS a) const;
    bool anyDefinedValues() const;
    void clearDefinedArea();
    void addDefinedArea(ADDRESS from,ADDRESS to);
private:
    SectionInfoImpl *Impl;
};


#endif // SECTIONINFO_H
