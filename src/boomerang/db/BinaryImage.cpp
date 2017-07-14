#include "BinaryImage.h"

#include "boomerang/util/types.h"

#include <QDebug>
#include <algorithm>


BinaryImage::BinaryImage()
{
}


BinaryImage::~BinaryImage()
{
}


void BinaryImage::reset()
{
	m_sectionMap.clear();

	for (IBinarySection *si : m_sections) {
		delete si;
	}

	m_sections.clear();
}


Byte BinaryImage::readNative1(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		qDebug() << "Target Memory access in unmapped section " << nat.toString();
		return -1;
	}

	HostAddress host = si->getHostAddr() - si->getSourceAddr() + nat;
	return *(Byte *)host.value();
}


SWord BinaryImage::readNative2(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	HostAddress host = si->getHostAddr() - si->getSourceAddr() + nat;
	return Util::readWord((const void*)host.value(), si->getEndian());
}


DWord BinaryImage::readNative4(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	HostAddress host = si->getHostAddr() - si->getSourceAddr() + nat;
	return Util::readDWord((const void*)host.value(), si->getEndian());
}


QWord BinaryImage::readNative8(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	HostAddress host = si->getHostAddr() - si->getSourceAddr() + nat;
	return Util::readQWord((const void*)host.value(), si->getEndian());
}


float BinaryImage::readNativeFloat4(Address nat)
{
	DWord raw = readNative4(nat);
	return *(float *)&raw; // Note: cast, not convert
}


double BinaryImage::readNativeFloat8(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	QWord raw = readNative8(nat);
	return *(double *)&raw;
}


void BinaryImage::writeNative4(Address nat, uint32_t n)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		qDebug() << "Write outside section";
		return;
	}

	HostAddress host  = si->getHostAddr() - si->getSourceAddr() + nat;

	Util::writeDWord((void*)host.value(), n, si->getEndian());
}


void BinaryImage::calculateTextLimits()
{
	m_limitTextLow  = Address::INVALID;
	m_limitTextHigh = Address::ZERO;
	m_textDelta     = 0;

	for (IBinarySection *pSect : m_sections) {
		if (!pSect->isCode()) {
			continue;
		}

		// The .plt section is an anomaly. It's code, but we never want to
		// decode it, and in Sparc ELF files, it's actually in the data
		// section (so it can be modified). For now, we make this ugly
		// exception
		if (".plt" == pSect->getName()) {
			continue;
		}

		if (pSect->getSourceAddr() < m_limitTextLow) {
			m_limitTextLow = pSect->getSourceAddr();
		}

		      Address hiAddress = pSect->getSourceAddr() + pSect->getSize();

		if (hiAddress > m_limitTextHigh) {
			m_limitTextHigh = hiAddress;
		}

		ptrdiff_t host_native_diff = (pSect->getHostAddr() - pSect->getSourceAddr()).value();

		if (m_textDelta == 0) {
			m_textDelta = host_native_diff;
		}
		else if (m_textDelta != host_native_diff) {
			fprintf(stderr, "warning: textDelta different for section %s (ignoring).\n", qPrintable(pSect->getName()));
		}
	}
}


const IBinarySection *BinaryImage::getSectionInfoByAddr(Address uEntry) const
{
	auto iter = m_sectionMap.find(uEntry);

	if (iter == m_sectionMap.end()) {
		return nullptr;
	}

	return iter->second;
}


int BinaryImage::getSectionIndexByName(const QString& sName)
{
	for (size_t i = 0; i < m_sections.size(); i++) {
		if (m_sections[i]->getName() == sName) {
			return i;
		}
	}

	return -1;
}


IBinarySection *BinaryImage::getSectionInfoByName(const QString& sName)
{
	int i = getSectionIndexByName(sName);

	if (i == -1) {
		return nullptr;
	}

	return m_sections[i];
}


bool BinaryImage::isReadOnly(Address uEntry)
{
	const SectionInfo *p = static_cast<const SectionInfo *>(getSectionInfoByAddr(uEntry));

	if (!p) {
		return false;
	}

	if (p->isReadOnly()) {
		return true;
	}

	QVariant v = p->attributeInRange("ReadOnly", uEntry, uEntry + 1);
	return !v.isNull();
}


Address BinaryImage::getLimitTextLow()
{
	auto interval = m_sectionMap.begin()->first;

	return interval.lower();
}


Address BinaryImage::getLimitTextHigh()
{
	auto interval = m_sectionMap.rbegin()->first;

	return interval.upper();
}


SectionInfo *BinaryImage::createSection(const QString& name, Address from, Address to)
{
	if (from == to) {
		to += 1; // open interval, so -> [from,to+1) is right
	}

#ifdef DEBUG
	// see https://stackoverflow.com/questions/25501044/gcc-ld-overlapping-sections-tbss-init-array-in-statically-linked-elf-bin
	// Basically, the .tbss section is of type SHT_NOBITS, so there is no data associated to the section.
	// It can therefore overlap other sections containing data.
	// This is a quirk of ELF programs linked statically with glibc
	if (name != ".tbss") {
		auto clash_with = m_sectionMap.find(boost::icl::interval<ADDRESS>::right_open(from, to));

		if ((clash_with != m_sectionMap.end()) && ((*clash_with->second).getName() != ".tbss")) {
			qDebug() << "Segment" << name << "would intersect existing one" << (*clash_with->second).getName();
			return nullptr;
		}
	}
#endif

	SectionInfo *sect = new SectionInfo(from, (to - from).value(), name);
	m_sections.push_back(sect);

	m_sectionMap.insert(std::make_pair(boost::icl::interval<Address>::right_open(from, to), sect));
	return sect;
}
