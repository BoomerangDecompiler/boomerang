#pragma once

/***************************************************************************/ /**
 * \file       IBinarySection.h
 *   Interface definition for Sections
 ******************************************************************************/
#include "boomerang/type/Type.h"

#include <cstdint>

class QVariant;
class QString;

class IBinarySection
{
public:
    virtual ~IBinarySection() {}
    virtual HostAddress     getHostAddr() const           = 0; ///< address of this section's data in the allocated memory
    virtual Address         getSourceAddr() const         = 0; ///< section's address in Source machine's 'coordinates'
    virtual uint32_t        getSize() const               = 0;
    virtual uint8_t         getEndian() const             = 0;
    virtual bool            isReadOnly() const            = 0;
    virtual bool            isCode() const                = 0;
    virtual bool            isData() const                = 0;
    virtual QString         getName() const               = 0;
    virtual bool            isAddressBss(Address a) const = 0;
    virtual bool            anyDefinedValues() const      = 0;
    virtual uint32_t        getEntrySize() const          = 0;

    virtual IBinarySection& setBss(bool)      = 0;
    virtual IBinarySection& setCode(bool)     = 0;
    virtual IBinarySection& setData(bool)     = 0;
    virtual IBinarySection& setReadOnly(bool) = 0;

    virtual IBinarySection& setHostAddr(HostAddress) = 0;
    virtual IBinarySection& setSourceAddr(Address)   = 0;
    virtual IBinarySection& setEntrySize(uint32_t)   = 0;
    virtual IBinarySection& setEndian(uint8_t)       = 0;

    virtual void            resize(uint32_t) = 0;
    virtual void            addDefinedArea(Address from, Address to) = 0;
    virtual void            setAttributeForRange(const QString& name, const QVariant& val, Address from, Address to) = 0;
    virtual QVariantMap     getAttributesForRange(Address from, Address to) = 0;

    ///////////////////
    // utility methods
    ///////////////////
    bool                    inSection(Address uAddr) const
    {
        return (uAddr >= getSourceAddr()) || (uAddr < getSourceAddr() + getSize());
    }
};
