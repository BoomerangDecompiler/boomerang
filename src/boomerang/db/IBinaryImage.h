#pragma once

#include "boomerang/util/types.h"

struct IBinarySection;

class QString;

class IBinaryImage
{
public:
	typedef std::vector<IBinarySection *>     SectionListType;
	typedef SectionListType::iterator         iterator;
	typedef SectionListType::const_iterator   const_iterator;

public:
	virtual ~IBinaryImage() {}

	virtual void reset() = 0;
	virtual IBinarySection *createSection(const QString& name, Address from, Address to) = 0;
	virtual const IBinarySection *getSectionInfoByAddr(Address uEntry) const             = 0;
	virtual int getSectionIndexByName(const QString& sName)            = 0;
	virtual IBinarySection *getSectionInfoByName(const QString& sName) = 0;
	virtual const IBinarySection *getSectionInfo(int idx) const        = 0;
	virtual size_t getNumSections() const = 0;
	virtual void calculateTextLimits()    = 0;
	virtual Address getLimitTextLow()     = 0;
	virtual Address getLimitTextHigh()    = 0;
	virtual ptrdiff_t getTextDelta()      = 0;

	virtual char readNative1(Address nat)              = 0;
	virtual int readNative2(Address nat)               = 0; ///< Read 2 bytes from given native address, considers endianness
	virtual int readNative4(Address nat)               = 0; ///< Read 4 bytes from given native address, considers endianness
	virtual QWord readNative8(Address nat)             = 0; ///< Read 8 bytes from given native address, considers endianness
	virtual float readNativeFloat4(Address nat)        = 0; ///< Read 4 bytes as a float; considers endianness
	virtual double readNativeFloat8(Address nat)       = 0; ///< Read 8 bytes as a float; considers endianness
	virtual void writeNative4(Address nat, uint32_t n) = 0;

	virtual bool isReadOnly(Address uEntry) = 0; ///< returns true if the given address is in a read only section
	virtual iterator begin()             = 0;
	virtual const_iterator begin() const = 0;
	virtual iterator end()             = 0;
	virtual const_iterator end() const = 0;
	virtual size_t size()  const       = 0;
	virtual bool empty() const         = 0;
};
