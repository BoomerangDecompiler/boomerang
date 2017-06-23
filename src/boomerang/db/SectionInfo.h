#pragma once

#include "boomerang/util/types.h"
#include "boomerang/db/IBinarySection.h"

#include <QString>

class QVariant;
struct SectionInfoImpl;

/// SectionInfo structure - All information about the sections is contained in these
/// structures.
struct SectionInfo : public IBinarySection
{
private:
	SectionInfo& operator=(const SectionInfo& other);


	QString         m_sectionName;          ///< Name of section
	ADDRESS         m_nativeAddr;           ///< Logical or native load address
	ADDRESS         m_hostAddr;             ///< Host or actual address of data
	uint32_t        m_sectionSize;          ///< Size of section in bytes
	uint32_t        m_sectionEntrySize;     ///< Size of one section entry (if applicable)
	unsigned        m_type;                 ///< Type of section (format dependent)
	unsigned        m_code     : 1;         ///< Set if section contains instructions
	unsigned        m_data     : 1;         ///< Set if section contains data
	unsigned        m_bss      : 1;         ///< Set if section is BSS (allocated only)
	unsigned        m_readOnly : 1;         ///< Set if this is a read only section
	uint8_t         m_endianness;           ///< 0 Little endian, 1 Big endian

public:
	SectionInfo(ADDRESS sourceAddr, uint32_t size, const QString& name = ""); // Constructor
	SectionInfo(const SectionInfo& other);
	virtual ~SectionInfo();

	ADDRESS getHostAddr()   const override { return m_hostAddr; }
	ADDRESS getSourceAddr() const override { return m_nativeAddr; }
	uint8_t getEndian()     const override { return m_endianness; }
	bool isReadOnly()    const override { return m_readOnly; }
	bool isCode()        const override { return m_code; }
	bool isData()        const override { return m_data; }
	uint32_t getSize()       const override { return m_sectionSize; }
	QString getName()       const override { return m_sectionName; }
	uint32_t getEntrySize()  const override { return m_sectionEntrySize; }

	IBinarySection& setBss(bool v) override { m_bss = v; return *this; }
	IBinarySection& setCode(bool v) override { m_code = v;  return *this; }
	IBinarySection& setData(bool v) override { m_data = v; return *this; }
	IBinarySection& setReadOnly(bool v) override { m_readOnly = v; return *this; }
	IBinarySection& setHostAddr(ADDRESS v) override { m_hostAddr = v; return *this; }
	IBinarySection& setSourceAddr(ADDRESS v) override { m_nativeAddr = v; return *this; }
	IBinarySection& setEntrySize(uint32_t v) override { m_sectionEntrySize = v; return *this; }
	IBinarySection& setEndian(uint8_t v) override { m_endianness = v; return *this; }

	/// Windows's PE file sections can contain any combination of code, data and bss.
	/// As such, it can't be correctly described by SectionInfo, why we need to override
	/// the behaviour of (at least) the question "Is this address in BSS".
	bool isAddressBss(ADDRESS a) const override;
	bool anyDefinedValues() const override;

	void            resize(uint32_t) override;

	void clearDefinedArea();
	void addDefinedArea(ADDRESS from, ADDRESS to) override;
	void setAttributeForRange(const QString& name, const QVariant& val, ADDRESS from, ADDRESS to) override;
	QVariantMap getAttributesForRange(ADDRESS from, ADDRESS to) override;
	QVariant attributeInRange(const QString& attrib, ADDRESS from, ADDRESS to) const;

private:
	SectionInfoImpl *m_impl;
};
