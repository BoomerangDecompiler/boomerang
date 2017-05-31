#pragma once

#include "types.h"

struct IBinarySection;
class QString;

class IBinaryImage {
public:
    typedef std::vector<IBinarySection *>    SectionListType;
    typedef SectionListType::iterator               iterator;
    typedef SectionListType::const_iterator   const_iterator;

    virtual ~IBinaryImage() {}
    virtual void reset() = 0;
    virtual IBinarySection *createSection(const QString &name, ADDRESS from, ADDRESS to)=0;
    virtual const IBinarySection *getSectionInfoByAddr(ADDRESS uEntry) const = 0;
    virtual int GetSectionIndexByName(const QString &sName) = 0;
    virtual IBinarySection *GetSectionInfoByName(const QString &sName) = 0;
    virtual const IBinarySection *GetSectionInfo(int idx) const = 0;
    virtual size_t GetNumSections() const = 0 ;
    virtual void calculateTextLimits() = 0;
    virtual ADDRESS getLimitTextLow() = 0;
    virtual ADDRESS getLimitTextHigh() = 0;
    virtual ptrdiff_t getTextDelta() = 0;

    virtual char readNative1(ADDRESS nat) = 0;
    virtual int readNative2(ADDRESS nat) = 0;   //!< Read 2 bytes from given native address, considers endianness
    virtual int readNative4(ADDRESS nat) = 0;   //!< Read 4 bytes from given native address, considers endianness
    virtual QWord readNative8(ADDRESS nat) = 0; //!< Read 8 bytes from given native address, considers endianness
    virtual float readNativeFloat4(ADDRESS nat) = 0;//!< Read 4 bytes as a float; considers endianness
    virtual double readNativeFloat8(ADDRESS nat) = 0;//!< Read 8 bytes as a float; considers endianness
    virtual void writeNative4(ADDRESS nat, uint32_t n)=0;

    virtual bool isReadOnly(ADDRESS uEntry) =0; //!< returns true if the given address is in a read only section
    virtual iterator                begin()       =0;
    virtual const_iterator          begin() const =0;
    virtual iterator                end  ()       =0;
    virtual const_iterator          end  () const =0;
    virtual size_t                  size()  const =0;
    virtual bool                    empty() const =0;

};
