#ifndef IBINARYSECTION_H
#define IBINARYSECTION_H
/***************************************************************************/ /**
  * \file       IBinarySection.h
  *   Interface definition for Sections
  ******************************************************************************/
#include "type.h"

#include <stdint.h>

class QVariant;
class QString;

class IBinarySection {
public:
    virtual ~IBinarySection() {}
    virtual ADDRESS hostAddr() const = 0; //!< address of this section's data in the allocated
    virtual ADDRESS sourceAddr() const = 0; //!< section's address in Source machine's 'coordinates'
    virtual uint32_t size() const = 0;
    virtual uint8_t getEndian() const = 0;
    virtual bool    isReadOnly() const = 0;
    virtual bool    isCode() const = 0;
    virtual bool    isData() const = 0;
    virtual QString getName() const = 0;
    virtual bool    isAddressBss(ADDRESS a) const = 0;
    virtual bool    anyDefinedValues() const  = 0;
    virtual uint32_t getEntrySize() const = 0;

    virtual IBinarySection &setBss(bool ) = 0;
    virtual IBinarySection &setCode(bool ) = 0;
    virtual IBinarySection &setData(bool ) = 0;
    virtual IBinarySection &setReadOnly(bool ) = 0;
    virtual IBinarySection &setHostAddr(ADDRESS ) = 0;
    virtual IBinarySection &setEntrySize(uint32_t ) = 0;
    virtual IBinarySection &setEndian(uint8_t ) = 0;

    virtual void    resize(uint32_t ) = 0;
    virtual void    addDefinedArea(ADDRESS from,ADDRESS to) = 0;
    virtual void    setAttributeForRange(const QString &name,const QVariant &val,ADDRESS from,ADDRESS to) = 0;
    // utility methods
    bool inSection(ADDRESS uAddr) const {
        return (uAddr >= sourceAddr()) || (uAddr < sourceAddr() + size());
    }

};

#endif // IBINARYSECTION_H
