#pragma once

#include "SectionInfo.h"
#include "db/IBinaryImage.h"

#include <boost/icl/interval_map.hpp>


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
	SectionHolder operator+=(const SectionHolder& /*s*/)
	{
		throw std::runtime_error("Cannot aggregate SectionInfos !");
	}
};

struct BinaryImage : public IBinaryImage
{
protected:
	BinaryImage(const BinaryImage&);            // prevent copy-construction
	BinaryImage& operator=(const BinaryImage&); // prevent assignment

public:
	/// The type for the list of functions.
	typedef boost::icl::interval_map<Address, SectionHolder> MapAddressRangeToSection;

public:
	BinaryImage();
	~BinaryImage();

	// IBinaryImage interface
	void reset() override;

	size_t getNumSections() const override { return m_sections.size(); }
	   Address                  imageToSource(Address); /// convert image address ( host pointer into image data ) to valid Source machine ADDRESS
	   Address                  sourceToImage(Address); /// convert Source machine ADDRESS into valid image ADDRESS

	char readNative1(Address nat) override;
	int readNative2(Address nat) override;
	int readNative4(Address nat) override;
	QWord readNative8(Address nat) override;

	/// Read 4 bytes as a float
	float readNativeFloat4(Address nat) override;

	/// Read 8 bytes as a double value
	double readNativeFloat8(Address nat) override;
	void writeNative4(Address nat, uint32_t n) override;
	void calculateTextLimits() override;

	/// Find the section, given an address in the section
	const IBinarySection *getSectionInfoByAddr(Address uEntry) const override;

	/// Find section index given name, or -1 if not found
	int getSectionIndexByName(const QString& sName) override;

	IBinarySection *getSectionInfoByName(const QString& sName) override;

	const IBinarySection *getSectionInfo(int idx) const override { return m_sections[idx]; }
	bool isReadOnly(Address uEntry) override;
	   Address getLimitTextLow() override;
	   Address getLimitTextHigh() override;

	ptrdiff_t getTextDelta() override { return m_textDelta; }

	SectionInfo *createSection(const QString& name, Address from, Address to) override;

	iterator begin()             override { return m_sections.begin(); }
	const_iterator begin() const override { return m_sections.begin(); }
	iterator end()               override { return m_sections.end(); }
	const_iterator end()   const override { return m_sections.end(); }

	size_t size() const override { return m_sections.size(); }
	bool empty()  const override { return m_sections.empty(); }

private:
	   Address                  m_limitTextLow;
	   Address                  m_limitTextHigh;
	ptrdiff_t                m_textDelta;
	MapAddressRangeToSection m_sectionMap;
	SectionListType          m_sections; ///< The section info
};
