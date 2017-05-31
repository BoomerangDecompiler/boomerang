#pragma once

#include "SectionInfo.h"
#include "IBinaryImage.h"

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
	typedef boost::icl::interval_map<ADDRESS, SectionHolder> MapAddressRangeToSection;

public:
	BinaryImage();
	~BinaryImage();
	// IBinaryImage interface
	void reset() override;

	size_t GetNumSections() const override { return Sections.size(); }
	ADDRESS                  imageToSource(ADDRESS); //! convert image address ( host pointer into image data ) to valid Source machine ADDRESS
	ADDRESS                  sourceToImage(ADDRESS); //! convert Source machine ADDRESS into valid image ADDRESS

	char readNative1(ADDRESS nat) override;
	int readNative2(ADDRESS nat) override;
	int readNative4(ADDRESS nat) override;
	QWord readNative8(ADDRESS nat) override;
	float readNativeFloat4(ADDRESS nat) override;
	double readNativeFloat8(ADDRESS nat) override;
	void writeNative4(ADDRESS nat, uint32_t n) override;
	void calculateTextLimits() override;

	//! Find the section, given an address in the section
	const IBinarySection *getSectionInfoByAddr(ADDRESS uEntry) const override;

	int GetSectionIndexByName(const QString& sName) override;
	IBinarySection *GetSectionInfoByName(const QString& sName) override;

	const IBinarySection *GetSectionInfo(int idx) const override { return Sections[idx]; }
	bool isReadOnly(ADDRESS uEntry) override;
	ADDRESS getLimitTextLow() override;
	ADDRESS getLimitTextHigh() override;

	ptrdiff_t getTextDelta() override { return TextDelta; }

	SectionInfo *createSection(const QString& name, ADDRESS from, ADDRESS to) override;

	iterator begin()       override { return Sections.begin(); }
	const_iterator begin() const override { return Sections.begin(); }
	iterator end()       override { return Sections.end(); }
	const_iterator end() const override { return Sections.end(); }
	size_t size()  const override { return Sections.size(); }
	bool empty() const override { return Sections.empty(); }

private:
	ADDRESS                  limitTextLow;
	ADDRESS                  limitTextHigh;
	ptrdiff_t                TextDelta;
	MapAddressRangeToSection SectionMap;
	SectionListType          Sections; //!< The section info
};
