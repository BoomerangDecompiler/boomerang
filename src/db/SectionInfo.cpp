#include "SectionInfo.h"
#include "core/log.h"
#include "db/IBinaryImage.h"

#include <QVariantMap>
#include <QDebug>

#include <boost/icl/interval_set.hpp>
#include <boost/icl/interval_map.hpp>

#include <algorithm>
#include <utility>

struct VariantHolder
{
	mutable QVariantMap val;
	QVariantMap&        get() const { return val; }
	VariantHolder& operator+=(const VariantHolder& other)
	{
		val = val.unite(other.val);
		return *this;
	}

	bool operator==(const VariantHolder& other) const
	{
		return val == other.val;
	}
};

struct SectionInfoImpl
{
	boost::icl::interval_set<ADDRESS>                m_hasDefinedValue;
	boost::icl::interval_map<ADDRESS, VariantHolder> m_attributeMap;

public:
	void clearDefinedArea()
	{
		m_hasDefinedValue.clear();
	}

	void                                             addDefinedArea(ADDRESS from, ADDRESS to)
	{
		m_hasDefinedValue.insert(boost::icl::interval<ADDRESS>::right_open(from, to));
	}

	bool                                             isAddressBss(ADDRESS a) const
	{
		assert(!m_hasDefinedValue.empty());
		return m_hasDefinedValue.find(a) == m_hasDefinedValue.end();
	}

	void                                             setAttributeForRange(const QString& name, const QVariant& val, ADDRESS from, ADDRESS to)
	{
		QVariantMap vmap;

		vmap[name] = val;
		VariantHolder map { vmap };
		m_attributeMap.add(std::make_pair(boost::icl::interval<ADDRESS>::right_open(from, to), map));
	}

	QVariant                                         attributeInRange(const QString& attrib, ADDRESS from, ADDRESS to) const
	{
		auto v = m_attributeMap.equal_range(boost::icl::interval<ADDRESS>::right_open(from, to));

		if (v.first == m_attributeMap.end()) {
			return QVariant();
		}

		QList<QVariant> vals;

		for (auto iter = v.first; iter != v.second; ++iter) {
			if (iter->second.get().contains(attrib)) {
				vals << iter->second.get()[attrib];
			}
		}

		if (vals.size() == 1) {
			return vals.front();
		}

		return QVariant(vals);
	}

	QVariantMap                                      getAttributesForRange(ADDRESS from, ADDRESS to)
	{
		QVariantMap res;
		auto        v = m_attributeMap.equal_range(boost::icl::interval<ADDRESS>::right_open(from, to));

		if (v.first == m_attributeMap.end()) {
			return res;
		}

		for (auto iter = v.first; iter != v.second; ++iter) {
			res.unite(iter->second.get());
		}

		return res;
	}
};


SectionInfo::SectionInfo(ADDRESS sourceAddr, uint32_t size, const QString& name)
	: m_sectionName(name)
	, m_nativeAddr(sourceAddr)
	, m_hostAddr(ADDRESS::g(0L))
	, m_sectionSize(size)
	, m_sectionEntrySize(0)
	, m_type(0)
	, m_code(false)
	, m_data(false)
	, m_bss(0)
	, m_readOnly(0)
	, m_impl(new SectionInfoImpl)
{
}


SectionInfo::SectionInfo(const SectionInfo& other)
	: SectionInfo(other.m_nativeAddr, other.m_sectionSize, other.m_sectionName)
{
	*m_impl            = *other.m_impl;
	m_hostAddr         = other.m_hostAddr;
	m_sectionEntrySize = other.m_sectionEntrySize;
	m_code             = other.m_code;
	m_data             = other.m_data;
	m_bss      = other.m_bss;
	m_readOnly = other.m_readOnly;
}


SectionInfo::~SectionInfo()
{
	delete m_impl;
}


bool SectionInfo::isAddressBss(ADDRESS a) const
{
	assert(a >= m_nativeAddr && a < m_nativeAddr + m_sectionSize);

	if (m_bss) {
		return true;
	}

	if (m_readOnly) {
		return false;
	}

	return m_impl->isAddressBss(a);
}


bool SectionInfo::anyDefinedValues() const
{
	return !m_impl->m_hasDefinedValue.empty();
}


void SectionInfo::resize(uint32_t sz)
{
	qDebug() << "SectionInfo::resize not fully implemented yet";
	m_sectionSize = sz;
//    assert(false && "This function is not implmented yet");
//    if(sz!=uSectionSize) {
//        const IBinarySection *sect = Boomerang::get()->getImage()->getSectionInfoByAddr(uNativeAddr+sz);
//        if(sect==nullptr || sect==this ) {
//        }
//    }
}


void SectionInfo::clearDefinedArea()
{
	m_impl->clearDefinedArea();
}


void SectionInfo::addDefinedArea(ADDRESS from, ADDRESS to)
{
	m_impl->addDefinedArea(from, to);
}


void SectionInfo::setAttributeForRange(const QString& name, const QVariant& val, ADDRESS from, ADDRESS to)
{
	m_impl->setAttributeForRange(name, val, from, to);
}


QVariantMap SectionInfo::getAttributesForRange(ADDRESS from, ADDRESS to)
{
	return m_impl->getAttributesForRange(from, to);
}


QVariant SectionInfo::attributeInRange(const QString& attrib, ADDRESS from, ADDRESS to) const
{
	return m_impl->attributeInRange(attrib, from, to);
}
