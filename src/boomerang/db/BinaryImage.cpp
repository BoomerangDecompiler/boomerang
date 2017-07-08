#include "BinaryImage.h"

#include "boomerang/util/types.h"

#include <QDebug>
#include <algorithm>

namespace
{
/***************************************************************************/ /**
 *
 * \brief    Read a 2 or 4 byte quantity from host address (C pointer) p
 * \note        Takes care of reading the correct endianness, set early on into m_elfEndianness
 * \param    ps or pi: host pointer to the data
 * \returns        An integer representing the data
 ******************************************************************************/
int Read2(const short *ps, bool bigEndian)
{
	const unsigned char *p = (const unsigned char *)ps;

	if (bigEndian) {
		// Big endian
		return (int)((p[0] << 8) + p[1]);
	}
	else {
		// Little endian
		return (int)(p[0] + (p[1] << 8));
	}
}


int Read4(const int *pi, bool bigEndian)
{
	const short *p = (const short *)pi;

	if (bigEndian) {
		return (int)((Read2(p, bigEndian) << 16) + Read2(p + 1, bigEndian));
	}
	else {
		return (int)(Read2(p, bigEndian) + (Read2(p + 1, bigEndian) << 16));
	}
}
}

void Write4(int *pi, int val, bool bigEndian)
{
	char *p = (char *)pi;

	if (bigEndian) {
		// Big endian
		*p++ = (char)(val >> 24);
		*p++ = (char)(val >> 16);
		*p++ = (char)(val >> 8);
		*p   = (char)val;
	}
	else {
		*p++ = (char)val;
		*p++ = (char)(val >> 8);
		*p++ = (char)(val >> 16);
		*p   = (char)(val >> 24);
	}
}


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


char BinaryImage::readNative1(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		qDebug() << "Target Memory access in unmapped section " << nat.toString();
		return -1;
	}

	Address host = si->getHostAddr() - si->getSourceAddr() + nat;
	return *(char *)host.value();
}


int BinaryImage::readNative2(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	   Address host = si->getHostAddr() - si->getSourceAddr() + nat;
	return Read2((short *)host.value(), si->getEndian());
}


int BinaryImage::readNative4(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	   Address host = si->getHostAddr() - si->getSourceAddr() + nat;
	return Read4((int *)host.value(), si->getEndian());
}


// Read 8 bytes from given native address
QWord BinaryImage::readNative8(Address nat)   // TODO: lifted from Win32 loader, likely wrong
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	QWord raw = 0;
#ifdef WORDS_BIGENDIAN   // This tests the  host     machine
	if (si->Endiannes) { // This tests the source machine
#else
	if (si->getEndian() == 0) {
#endif  // Balance }
		// Source and host are same endianness
		raw |= (QWord)readNative4(nat);
		raw |= (QWord)readNative4(nat + 4) << 32;
	}
	else {
		// Source and host are different endianness
		raw |= (QWord)readNative4(nat + 4);
		raw |= (QWord)readNative4(nat) << 32;
	}

	// return reinterpret_cast<long long>(*raw);       // Note: cast, not convert!!
	return raw;
}


float BinaryImage::readNativeFloat4(Address nat)
{
	int raw = readNative4(nat);

	// Ugh! gcc says that reinterpreting from int to float is invalid!!
	// return reinterpret_cast<float>(raw);      // Note: cast, not convert!!
	return *(float *)&raw; // Note: cast, not convert
}


double BinaryImage::readNativeFloat8(Address nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	int raw[2];
#ifdef WORDS_BIGENDIAN   // This tests the  host     machine
	if (si->Endiannes) { // This tests the source machine
#else
	if (si->getEndian() == 0) {
#endif  // Balance }
		// Source and host are same endianness
		raw[0] = readNative4(nat);
		raw[1] = readNative4(nat + 4);
	}
	else {
		// Source and host are different endianness
		raw[1] = readNative4(nat);
		raw[0] = readNative4(nat + 4);
	}

	// return reinterpret_cast<double>(*raw);    // Note: cast, not convert!!
	return *(double *)raw;
}


void BinaryImage::writeNative4(Address nat, uint32_t n)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		qDebug() << "Write outside section";
		return;
	}

	   Address host      = si->getHostAddr() - si->getSourceAddr() + nat;
	uint8_t *host_ptr = (unsigned char *)host.value();

	if (si->getEndian() == 1) {
		host_ptr[0] = (n >> 24) & 0xff;
		host_ptr[1] = (n >> 16) & 0xff;
		host_ptr[2] = (n >> 8) & 0xff;
		host_ptr[3] = n & 0xff;
	}
	else {
		host_ptr[3] = (n >> 24) & 0xff;
		host_ptr[2] = (n >> 16) & 0xff;
		host_ptr[1] = (n >> 8) & 0xff;
		host_ptr[0] = n & 0xff;
	}
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
	for (int32_t i = m_sections.size() - 1; i >= 0; --i) {
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
