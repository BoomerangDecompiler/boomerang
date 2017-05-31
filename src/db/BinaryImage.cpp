#include "BinaryImage.h"
#include "include/types.h"
#include "include/config.h"

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
	else{
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
	SectionMap.clear();

	for (IBinarySection *si : Sections) {
		delete si;
	}

	Sections.clear();
}


char BinaryImage::readNative1(ADDRESS nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		qDebug() << "Target Memory access in unmapped Section " << nat.m_value;
		return -1;
	}

	ADDRESS host = si->hostAddr() - si->sourceAddr() + nat;
	return *(char *)host.m_value;
}


int BinaryImage::readNative2(ADDRESS nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	ADDRESS host = si->hostAddr() - si->sourceAddr() + nat;
	return Read2((short *)host.m_value, si->getEndian());
}


int BinaryImage::readNative4(ADDRESS nat)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		return 0;
	}

	ADDRESS host = si->hostAddr() - si->sourceAddr() + nat;
	return Read4((int *)host.m_value, si->getEndian());
}


// Read 8 bytes from given native address
QWord BinaryImage::readNative8(ADDRESS nat)   // TODO: lifted from Win32 loader, likely wrong
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


// Read 4 bytes as a float
float BinaryImage::readNativeFloat4(ADDRESS nat)
{
	int raw = readNative4(nat);

	// Ugh! gcc says that reinterpreting from int to float is invalid!!
	// return reinterpret_cast<float>(raw);      // Note: cast, not convert!!
	return *(float *)&raw; // Note: cast, not convert
}


// Read 8 bytes as a float
double BinaryImage::readNativeFloat8(ADDRESS nat)
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


void BinaryImage::writeNative4(ADDRESS nat, uint32_t n)
{
	const IBinarySection *si = getSectionInfoByAddr(nat);

	if (si == nullptr) {
		qDebug() << "Write outside section";
		return;
	}

	ADDRESS host      = si->hostAddr() - si->sourceAddr() + nat;
	uint8_t *host_ptr = (unsigned char *)host.m_value;

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
	limitTextLow  = ADDRESS::g(0xFFFFFFFF);
	limitTextHigh = ADDRESS::g(0L);
	TextDelta     = 0;

	for (IBinarySection *pSect : Sections) {
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

		if (pSect->sourceAddr() < limitTextLow) {
			limitTextLow = pSect->sourceAddr();
		}

		ADDRESS hiAddress = pSect->sourceAddr() + pSect->size();

		if (hiAddress > limitTextHigh) {
			limitTextHigh = hiAddress;
		}

		ptrdiff_t host_native_diff = (pSect->hostAddr() - pSect->sourceAddr()).m_value;

		if (TextDelta == 0) {
			TextDelta = host_native_diff;
		}
		else {
			if (TextDelta != host_native_diff) {
				fprintf(stderr, "warning: textDelta different for section %s (ignoring).\n", qPrintable(pSect->getName()));
			}
		}
	}
}


const IBinarySection *BinaryImage::getSectionInfoByAddr(ADDRESS uEntry) const
{
	if (!uEntry.isSourceAddr()) {
		qDebug() << "getSectionInfoByAddr with non-Source ADDRESS";
	}

	auto iter = SectionMap.find(uEntry);

	if (iter == SectionMap.end()) {
		return nullptr;
	}

	return iter->second;
}


/// Find section index given name, or -1 if not found
int BinaryImage::GetSectionIndexByName(const QString& sName)
{
	for (int32_t i = Sections.size() - 1; i >= 0; --i) {
		if (Sections[i]->getName() == sName) {
			return i;
		}
	}

	return -1;
}


IBinarySection *BinaryImage::GetSectionInfoByName(const QString& sName)
{
	int i = GetSectionIndexByName(sName);

	if (i == -1) {
		return nullptr;
	}

	return Sections[i];
}


bool BinaryImage::isReadOnly(ADDRESS uEntry)
{
	const SectionInfo *p = static_cast<const SectionInfo *>(getSectionInfoByAddr(uEntry));

	if (!p) {
		return false;
	}

	if (p->bReadOnly) {
		return true;
	}

	QVariant v = p->attributeInRange("ReadOnly", uEntry, uEntry + 1);
	return !v.isNull();
}


ADDRESS BinaryImage::getLimitTextLow()
{
	auto interval = SectionMap.begin()->first;

	return interval.lower();
}


ADDRESS BinaryImage::getLimitTextHigh()
{
	auto interval = SectionMap.rbegin()->first;

	return interval.upper();
}


SectionInfo *BinaryImage::createSection(const QString& name, ADDRESS from, ADDRESS to)
{
	if (from == to) {
		to += 1; // open interval, so -> [from,to+1) is right
	}

//    for(auto iter = SectionMap.begin(),e=SectionMap.end(); iter!=e; ++iter) {
//        qDebug() << iter->first.lower().toString() << " - "<< iter->first.upper().toString();
//    }
	auto clash_with = SectionMap.find(boost::icl::interval<ADDRESS>::right_open(from, to));

	if (clash_with != SectionMap.end()) {
		qDebug() << "Segment" << name << "would intersect existing one" << (*clash_with->second).getName();
		return nullptr;
	}

	SectionInfo *sect = new SectionInfo(name);
	sect->uNativeAddr  = from;
	sect->uSectionSize = (to - from).m_value;
	Sections.push_back(sect);

	SectionMap.add(std::make_pair(boost::icl::interval<ADDRESS>::right_open(from, to), sect));
	return sect;
}
